//
//  react-native-turbo-base64.cpp
//  react-native-turbo-base64
//
//  Created by Oleksandr Shumihin on 7/2/26.
//

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <cstring>

#include "react-native-turbo-base64.h"
#include "base64.h"

using namespace facebook;

void rntb_base64::install(jsi::Runtime* runtime) {
  jsi::Runtime& rt = *runtime;
  auto decodeNameId = jsi::PropNameID::forAscii(rt, "decodeBase64ToArrayBuffer");
  auto encodeNameId = jsi::PropNameID::forAscii(rt, "encodeBase64FromArrayBuffer");

  auto decode = jsi::Function::createFromHostFunction(
      rt, decodeNameId, 2,
      [](jsi::Runtime& runtime, const jsi::Value&, const jsi::Value* arguments,
         size_t count) -> jsi::Value {
        if (count < 1 || (count > 0 && !arguments[0].isString())) [[unlikely]] {
          throw jsi::JSError(
              runtime, "base64ToArrayBuffer: first argument must be a string");
        }

        try {
          std::string input = arguments[0].getString(runtime).utf8(runtime);

          bool removeLinebreaks = false;
          if (count > 1 && arguments[1].isBool()) {
            removeLinebreaks = arguments[1].asBool();
          }

          if (removeLinebreaks) [[unlikely]] {
            // Single-pass removal of both \n and \r
            auto new_end = std::remove_if(
                input.begin(), input.end(),
                [](char c) { return c == '\n' || c == '\r'; });
            input.erase(new_end, input.end());
          }

          // Add padding if needed (for URL-safe base64 without padding)
          const size_t remainder = input.size() % 4;
          if (remainder > 0) [[unlikely]] {
            input.append(4 - remainder, '=');
          }

          const char* input_data = input.data();
          const size_t input_size = input.size();

          // Fast path check: detect padding in middle (rare case)
          bool has_middle_padding = false;
          if (input_size > 4) {
            // Check if '=' appears before the last 4 chars
            const char* padding_ptr = static_cast<const char*>(
                std::memchr(input_data, '=', input_size - 4));
            has_middle_padding = (padding_ptr != nullptr);
          }

          jsi::Function arrayBufferCtor =
              runtime.global().getPropertyAsFunction(runtime, "ArrayBuffer");

          // Fast path: normal single-block decoding (99.9% of cases)
          if (!has_middle_padding) [[likely]] {
            const size_t output_length =
                base64_decoded_length(input_data, input_size);
            if (output_length == 0 && input_size > 0) [[unlikely]] {
              throw jsi::JSError(runtime,
                                 "Input is not valid base64-encoded data");
            }

            jsi::Object o = arrayBufferCtor
                                .callAsConstructor(
                                    runtime, static_cast<int>(output_length))
                                .getObject(runtime);
            jsi::ArrayBuffer buf = o.getArrayBuffer(runtime);

            if (base64_decode_fast(input_data, input_size, buf.data(runtime),
                                   output_length)) [[likely]] {
              return o;
            }

            throw jsi::JSError(runtime,
                               "Input is not valid base64-encoded data");
          }

          // Slow path: multi-block decoding for padding in middle (rare)
          std::vector<uint8_t> result;
          result.reserve(
              (input_size * 3) / 4);  // Pre-allocate approximate size
          size_t pos = 0;

          while (pos + 4 <= input_size) {
            // Check if current block has padding
            const bool has_padding = (input_data[pos + 2] == '=' ||
                                      input_data[pos + 3] == '=');

            // Find end of current segment (up to next padding or end)
            size_t segment_end = pos + 4;
            if (has_padding) {
              // This block has padding, decode it
              // Inline length calculation for 4-byte blocks (faster than function call)
              const size_t block_output_len = 3 -
                  (input_data[pos + 3] == '=') - (input_data[pos + 2] == '=');
              if (block_output_len > 0) [[likely]] {
                const size_t old_size = result.size();
                result.resize(old_size + block_output_len);

                if (!base64_decode_fast(input_data + pos, 4,
                                        result.data() + old_size,
                                        block_output_len)) {
                  throw jsi::JSError(runtime,
                                     "Input is not valid base64-encoded data");
                }
              }
              pos += 4;
            } else {
              // Decode multiple non-padding blocks together for efficiency
              while (segment_end + 4 <= input_size &&
                     input_data[segment_end + 2] != '=' &&
                     input_data[segment_end + 3] != '=') {
                segment_end += 4;
              }

              const size_t segment_size = segment_end - pos;
              const size_t segment_output_len =
                  base64_decoded_length(input_data + pos, segment_size);
              if (segment_output_len > 0) {
                const size_t old_size = result.size();
                result.resize(old_size + segment_output_len);

                if (!base64_decode_fast(input_data + pos, segment_size,
                                        result.data() + old_size,
                                        segment_output_len)) {
                  throw jsi::JSError(runtime,
                                     "Input is not valid base64-encoded data");
                }
              }
              pos = segment_end;
            }
          }

          jsi::Object o =
              arrayBufferCtor
                  .callAsConstructor(runtime, static_cast<int>(result.size()))
                  .getObject(runtime);
          jsi::ArrayBuffer buf = o.getArrayBuffer(runtime);
          std::memcpy(buf.data(runtime), result.data(), result.size());
          return o;
        } catch (const std::runtime_error& error) {
          throw jsi::JSError(runtime, error.what());
        } catch (const jsi::JSError&) {
          throw;
        } catch (...) {
          throw jsi::JSError(runtime, "unknown decoding error");
        }
      });

  auto encode = jsi::Function::createFromHostFunction(
      rt, encodeNameId,
      2,  // data, url
      [](jsi::Runtime& runtime, const jsi::Value& thisValue,
         const jsi::Value* arguments, size_t count) -> jsi::Value {
        if (count == 0) [[unlikely]] {
          throw jsi::JSError(
              runtime, "base64FromArrayBuffer: requires at least 1 argument");
        }

        try {
          if (!arguments[0].isObject() ||
              !arguments[0].asObject(runtime).isArrayBuffer(runtime)) [[unlikely]] {
            throw jsi::JSError(runtime, "encodeBase64FromArrayBuffer: first argument must be an ArrayBuffer");
          }
          
          bool url = false;
          if (count > 1 && arguments[1].isBool()) {
            url = arguments[1].asBool();
          }

          jsi::ArrayBuffer input_array_buffer = arguments[0].asObject(runtime).getArrayBuffer(runtime);
          const uint8_t* input_data = input_array_buffer.data(runtime);
          const size_t input_size = input_array_buffer.size(runtime);

          // Calculate output size
          const size_t output_size = base64_encoded_length(input_size, url);

          // Allocate output buffer
          std::string result;
          result.resize(output_size);

          // Encode
          const size_t actual_size = base64_encode_fast(
              input_data, input_size, &result[0], output_size, url);

          if (actual_size == 0 && input_size > 0) {
            throw jsi::JSError(runtime,
                               "base64FromArrayBuffer: encoding failed");
          }

          // Resize to actual size (important for URL-safe which may skip
          // padding)
          result.resize(actual_size);

          return jsi::String::createFromUtf8(runtime, result);
        } catch (const jsi::JSError&) {
          throw;
        } catch (const std::runtime_error& error) {
          throw jsi::JSError(runtime, error.what());
        } catch (...) {
          throw jsi::JSError(runtime, "unknown encoding error");
        }
      });

  rt.global().setProperty(rt, decodeNameId, std::move(decode));
  rt.global().setProperty(rt, encodeNameId, std::move(encode));
}
