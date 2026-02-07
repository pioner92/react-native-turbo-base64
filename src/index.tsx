import { NativeModules } from 'react-native'

declare global {
  var encodeBase64FromArrayBuffer: (input: ArrayBuffer, urlSafe: boolean) => string;
  var decodeBase64ToArrayBuffer: (input: string, removeLinebreaks: boolean) => Uint8Array;
}


let RNTurboBase64Initialized = !!globalThis.encodeBase64FromArrayBuffer;

if (!RNTurboBase64Initialized) {
  if (NativeModules.RNTurboBase64) {
    NativeModules.RNTurboBase64.install();
    RNTurboBase64Initialized = !!globalThis.encodeBase64FromArrayBuffer;
    console.log('✅ react-native-turbo-base64 initialized successfully')
  }
}

/**
 * Calculates valid length and placeholder length for base64 string
 */
function getLens(b64: string) {
  const len = b64.length

  if (len % 4 > 0) {
    throw new Error('Invalid string. Length must be a multiple of 4')
  }

  let validLen = b64.indexOf('=')
  if (validLen === -1) validLen = len

  const placeHoldersLen = validLen === len ? 0 : 4 - (validLen % 4)

  return [validLen, placeHoldersLen] as const
}


/**
 * Calculates the byte length of a base64 string
 */
export function byteLength(b64: string) {
  const [validLen, placeHoldersLen] = getLens(b64)
  return ((validLen + placeHoldersLen) * 3) / 4 - placeHoldersLen
}

/**
 * Removes padding characters from base64 string
 */
export const trimBase64Padding = (str: string) => {
  return str.replace(/[.=]{1,2}$/, '')
}

export function fromByteArray(uint8: Uint8Array,   urlSafe: boolean = false): string {
  if (uint8.buffer.byteLength > uint8.byteLength || uint8.byteOffset > 0) {
    const buffer =
      uint8.buffer instanceof ArrayBuffer
        ? uint8.buffer.slice(
          uint8.byteOffset,
          uint8.byteOffset + uint8.byteLength
        )
        : new ArrayBuffer(uint8.byteLength)

    if (buffer instanceof ArrayBuffer) {
      return global.encodeBase64FromArrayBuffer(buffer, urlSafe)
    }
  }

  const buffer =
    uint8.buffer instanceof ArrayBuffer
      ? uint8.buffer
      : new ArrayBuffer(uint8.byteLength)
  return global.encodeBase64FromArrayBuffer(buffer, urlSafe)

}

export function toByteArray(input: string,  removeLinebreaks: boolean = false): Uint8Array {
  if (typeof input !== 'string') {
    throw new TypeError('Input must be a string');
  }
  if (!input.length) {
    return new Uint8Array(0);
  }
  return new Uint8Array(globalThis.decodeBase64ToArrayBuffer(input, removeLinebreaks));
}


/**
 * Returns native base64 functions
 */
export const getNative = () => ({
  encodeBase64FromArrayBuffer: globalThis.encodeBase64FromArrayBuffer,
  decodeBase64ToArrayBuffer: globalThis.decodeBase64ToArrayBuffer
})
