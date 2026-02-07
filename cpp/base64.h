//
//  base64_o.h
// Created by Oleksandr Shumihin on 4/2/26.
//
#pragma once
#include <cstring>
#include <cstdint>


namespace rntb_base64 {
static constexpr uint8_t kInvalid = 0xFF;

// Encoding tables for standard and URL-safe base64
alignas(64) static constexpr char encoding_table_standard[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

alignas(64) static constexpr char encoding_table_url[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '-', '_'
};

// Cache line alignment for better performance
alignas(64) static constexpr uint8_t decoding_table[256] = {
  /* 0x00–0x0F */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  /* 0x10–0x1F */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  /* 0x20–0x2F */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,62  ,0xFF,62  ,0xFF,63  ,
  /* 0x30–0x3F */
  52  ,53  ,54  ,55  ,56  ,57  ,58  ,59  , 60  ,61  ,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  /* 0x40–0x4F */
  0xFF,0   ,1   ,2   ,3   ,4   ,5   ,6   , 7   ,8   ,9   ,10  ,11  ,12  ,13  ,14  ,
  /* 0x50–0x5F */
  15  ,16  ,17  ,18  ,19  ,20  ,21  ,22  , 23  ,24  ,25  ,0xFF,0xFF,0xFF,0xFF,63  ,
  /* 0x60–0x6F */
  0xFF,26  ,27  ,28  ,29  ,30  ,31  ,32  , 33  ,34  ,35  ,36  ,37  ,38  ,39  ,40  ,
  /* 0x70–0x7F */
  41  ,42  ,43  ,44  ,45  ,46  ,47  ,48  , 49  ,50  ,51  ,0xFF,0xFF,0xFF,0xFF,0xFF,
  /* 0x80–0xFF */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};


inline size_t base64_decoded_length(const char* data, size_t n) noexcept {
    if (n < 4) return 0;
    if ((n & 3u) != 0) return 0;

    size_t out = (n / 4) * 3;
    if (data[n - 1] == '=') --out;
    if (data[n - 2] == '=') --out;
    return out;
}

inline bool base64_decode_fast(
  const char* __restrict data,
  const size_t input_length,
  uint8_t* __restrict out,
  const size_t output_length
) noexcept {
  if (!data || !out) return false;

  if ((input_length & 3u) != 0) return false;
  if (input_length == 0) return output_length == 0;
  if (input_length < 4) return false;

  const size_t expected = base64_decoded_length(data, input_length);
  if (expected != output_length) return false;

  const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
  uint8_t* dst = out;

  const uint8_t* const end = p + input_length;
  const uint8_t* const last = end - 4;

  while (p + 16 <= last) {
    // Prefetch next block for better cache locality
    #ifdef __GNUC__
    __builtin_prefetch(p + 32, 0, 0);
    #endif

    // Block 1
    const uint8_t a0 = decoding_table[p[0]];
    const uint8_t b0 = decoding_table[p[1]];
    const uint8_t c0 = decoding_table[p[2]];
    const uint8_t d0 = decoding_table[p[3]];

    // Block 2
    const uint8_t a1 = decoding_table[p[4]];
    const uint8_t b1 = decoding_table[p[5]];
    const uint8_t c1 = decoding_table[p[6]];
    const uint8_t d1 = decoding_table[p[7]];

    // Block 3
    const uint8_t a2 = decoding_table[p[8]];
    const uint8_t b2 = decoding_table[p[9]];
    const uint8_t c2 = decoding_table[p[10]];
    const uint8_t d2 = decoding_table[p[11]];

    // Block 4
    const uint8_t a3 = decoding_table[p[12]];
    const uint8_t b3 = decoding_table[p[13]];
    const uint8_t c3 = decoding_table[p[14]];
    const uint8_t d3 = decoding_table[p[15]];

    if ((a0 | b0 | c0 | d0 | a1 | b1 | c1 | d1 |
         a2 | b2 | c2 | d2 | a3 | b3 | c3 | d3) & 0x80) {
      return false;
    }

    const uint32_t t0 = (uint32_t(a0) << 18) | (uint32_t(b0) << 12) | (uint32_t(c0) << 6) | uint32_t(d0);
    const uint32_t t1 = (uint32_t(a1) << 18) | (uint32_t(b1) << 12) | (uint32_t(c1) << 6) | uint32_t(d1);
    const uint32_t t2 = (uint32_t(a2) << 18) | (uint32_t(b2) << 12) | (uint32_t(c2) << 6) | uint32_t(d2);
    const uint32_t t3 = (uint32_t(a3) << 18) | (uint32_t(b3) << 12) | (uint32_t(c3) << 6) | uint32_t(d3);

    dst[0]  = uint8_t(t0 >> 16); dst[1]  = uint8_t(t0 >> 8); dst[2]  = uint8_t(t0);
    dst[3]  = uint8_t(t1 >> 16); dst[4]  = uint8_t(t1 >> 8); dst[5]  = uint8_t(t1);
    dst[6]  = uint8_t(t2 >> 16); dst[7]  = uint8_t(t2 >> 8); dst[8]  = uint8_t(t2);
    dst[9]  = uint8_t(t3 >> 16); dst[10] = uint8_t(t3 >> 8); dst[11] = uint8_t(t3);

    p += 16;
    dst += 12;
  }

  while (p < last) {
    const uint8_t a = decoding_table[p[0]];
    const uint8_t b = decoding_table[p[1]];
    const uint8_t c = decoding_table[p[2]];
    const uint8_t d = decoding_table[p[3]];

    if ((a | b | c | d) & 0x80) return false;

    const uint32_t triple =
      (uint32_t(a) << 18) |
      (uint32_t(b) << 12) |
      (uint32_t(c) <<  6) |
      (uint32_t(d) <<  0);

    dst[0] = uint8_t(triple >> 16);
    dst[1] = uint8_t(triple >> 8);
    dst[2] = uint8_t(triple);

    p += 4;
    dst += 3;
  }

  const uint8_t c0 = p[0];
  const uint8_t c1 = p[1];
  const uint8_t c2 = p[2];
  const uint8_t c3 = p[3];

  const uint8_t a = decoding_table[c0];
  const uint8_t b = decoding_table[c1];

  if ((a | b) & 0x80) return false;

  if (c2 == '=' && c3 == '=') {
    const uint32_t triple = (uint32_t(a) << 18) | (uint32_t(b) << 12);
    dst[0] = uint8_t(triple >> 16);
    return (size_t)(dst - out + 1) == output_length;
  }

  if (c3 == '=') {
    if (c2 == '=') return false;
    const uint8_t c = decoding_table[c2];
    if (c & 0x80) return false;

    const uint32_t triple =
      (uint32_t(a) << 18) |
      (uint32_t(b) << 12) |
      (uint32_t(c) <<  6);

    dst[0] = uint8_t(triple >> 16);
    dst[1] = uint8_t(triple >> 8);
    return (size_t)(dst - out + 2) == output_length;
  }

  if (c2 == '=') return false;

  const uint8_t c = decoding_table[c2];
  const uint8_t d = decoding_table[c3];

  if ((c | d) & 0x80) return false;

  const uint32_t triple =
    (uint32_t(a) << 18) |
    (uint32_t(b) << 12) |
    (uint32_t(c) <<  6) |
    (uint32_t(d) <<  0);

  dst[0] = uint8_t(triple >> 16);
  dst[1] = uint8_t(triple >> 8);
  dst[2] = uint8_t(triple);

  return (size_t)(dst - out + 3) == output_length;
}

// ============================================================================
// ENCODING FUNCTIONS
// ============================================================================

inline size_t base64_encoded_length(size_t input_length, bool url) noexcept {
  // Standard base64: (n + 2) / 3 * 4 with padding
  // URL-safe base64: may skip padding
  size_t len = ((input_length + 2) / 3) * 4;
  return len;
}

inline size_t base64_encode_fast(
  const uint8_t* __restrict data,
  const size_t input_length,
  char* __restrict out,
  const size_t output_capacity,
  bool url = false
) noexcept {
  if (!data || !out) [[unlikely]] return 0;

  const size_t expected_length = base64_encoded_length(input_length, url);
  if (output_capacity < expected_length) [[unlikely]] return 0;

  if (input_length == 0) [[unlikely]] return 0;

  const char* encoding_table = url ? encoding_table_url : encoding_table_standard;
  static constexpr char padding_chars[2] = {'=', '\0'};
  const uint8_t* p = data;
  char* dst = out;

  const uint8_t* const end = data + input_length;
  const uint8_t* const main_end = end - (input_length % 3);

  // ---- Loop unrolling: process 4 blocks (12 bytes -> 16 chars) at a time ----
  while (p + 12 <= main_end) {
    // Block 1: bytes 0-2
    const uint32_t v0 = (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8) | uint32_t(p[2]);
    dst[0] = encoding_table[(v0 >> 18) & 0x3F];
    dst[1] = encoding_table[(v0 >> 12) & 0x3F];
    dst[2] = encoding_table[(v0 >> 6) & 0x3F];
    dst[3] = encoding_table[v0 & 0x3F];

    // Block 2: bytes 3-5
    const uint32_t v1 = (uint32_t(p[3]) << 16) | (uint32_t(p[4]) << 8) | uint32_t(p[5]);
    dst[4] = encoding_table[(v1 >> 18) & 0x3F];
    dst[5] = encoding_table[(v1 >> 12) & 0x3F];
    dst[6] = encoding_table[(v1 >> 6) & 0x3F];
    dst[7] = encoding_table[v1 & 0x3F];

    // Block 3: bytes 6-8
    const uint32_t v2 = (uint32_t(p[6]) << 16) | (uint32_t(p[7]) << 8) | uint32_t(p[8]);
    dst[8] = encoding_table[(v2 >> 18) & 0x3F];
    dst[9] = encoding_table[(v2 >> 12) & 0x3F];
    dst[10] = encoding_table[(v2 >> 6) & 0x3F];
    dst[11] = encoding_table[v2 & 0x3F];

    // Block 4: bytes 9-11
    const uint32_t v3 = (uint32_t(p[9]) << 16) | (uint32_t(p[10]) << 8) | uint32_t(p[11]);
    dst[12] = encoding_table[(v3 >> 18) & 0x3F];
    dst[13] = encoding_table[(v3 >> 12) & 0x3F];
    dst[14] = encoding_table[(v3 >> 6) & 0x3F];
    dst[15] = encoding_table[v3 & 0x3F];

    p += 12;
    dst += 16;
  }

  // ---- Process remaining full blocks (3 bytes -> 4 chars) ----
  while (p < main_end) {
    const uint32_t val = (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8) | uint32_t(p[2]);
    dst[0] = encoding_table[(val >> 18) & 0x3F];
    dst[1] = encoding_table[(val >> 12) & 0x3F];
    dst[2] = encoding_table[(val >> 6) & 0x3F];
    dst[3] = encoding_table[val & 0x3F];

    p += 3;
    dst += 4;
  }

  // ---- Handle remaining bytes (0-2 bytes) ----
  const size_t remaining = end - p;

  if (remaining == 2) [[unlikely]] {
    // 2 bytes remaining: encode to 3 chars + 1 padding (or no padding for URL)
    const uint32_t val = (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8);
    dst[0] = encoding_table[(val >> 18) & 0x3F];
    dst[1] = encoding_table[(val >> 12) & 0x3F];
    dst[2] = encoding_table[(val >> 6) & 0x3F];
    dst[3] = padding_chars[url];
    dst += 3 + !url;
  } else if (remaining == 1) [[unlikely]] {
    // 1 byte remaining: encode to 2 chars + 2 padding (or no padding for URL)
    const uint32_t val = uint32_t(p[0]) << 16;
    dst[0] = encoding_table[(val >> 18) & 0x3F];
    dst[1] = encoding_table[(val >> 12) & 0x3F];
    dst[2] = padding_chars[url];
    dst[3] = padding_chars[url];
    dst += 2 + (!url << 1);
  }

  // Return actual length written
  return static_cast<size_t>(dst - out);
}

}

