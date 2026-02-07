import { expect } from 'chai'
import {  getNative } from 'react-native-turbo-base64'
import { describe, it } from '../MochaRNAdapter'

describe('basics', () => {
  it('check native code availability', async () => {
    const { encodeBase64FromArrayBuffer, decodeBase64ToArrayBuffer } = getNative()
    expect(encodeBase64FromArrayBuffer).to.not.be.undefined
    expect(decodeBase64ToArrayBuffer).to.not.be.undefined
  })
})
