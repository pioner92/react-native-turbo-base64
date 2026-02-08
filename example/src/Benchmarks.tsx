/* global performance */
import { useState } from 'react'
import { StyleSheet, View, Text, Pressable, Platform } from 'react-native'
import { fromByteArray as fromByteArrayTurbo, toByteArray as toByteArrayTurbo } from 'react-native-turbo-base64'
import { toByteArray, fromByteArray } from 'react-native-quick-base64'
// 5.3 KB
import { data as smallData } from './image.json'
// 1.2 MB
import { data as bigData } from './image-large.json'



const round = (num: number, decimalPlaces = 0): string => {
  return num.toFixed(decimalPlaces)
}

const Benchmarks = () => {
  const [turbo64SmallResult, setTurbo64SmallResult] = useState<number>(0)
  const [isProcessing, setIsProcessing] = useState<boolean>(false)
  const [turbo64BigResult, setTurbo64BigResult] = useState<number>(0)
  const [nativeBase64SmallResult, setNativeBase64SmallResult] = useState<number>(0)
  const [nativeBase64BigResult, setNativeBase64BigResult] = useState<number>(0)

  const handleNativeBase64 =  (data: string) => {
    let dataToProcess = data
    // await sleep(1)

    const startTime = performance.now()

    for (let iter = 0; iter < 30; iter++) {
      const decoded = toByteArray(dataToProcess)
      dataToProcess = fromByteArray(decoded)
      if (dataToProcess !== data) {
        throw new Error('Data does not match')
      }
    }
    const finishedTime = performance.now()
    console.log('done! took', finishedTime - startTime, 'milliseconds')
    return finishedTime - startTime
  }

  const handleTurbo64Press = (data: string) => {
    let dataToProcess = data
    // await sleep(1)

    const startTime = performance.now()

    for (let iter = 0; iter < 30; iter++) {
      const decoded = toByteArrayTurbo(dataToProcess)
      dataToProcess = fromByteArrayTurbo(decoded)
      if (dataToProcess !== data) {
        throw new Error('Data does not match')
      }
    }
    const finishedTime = performance.now()
    console.log('done! took', finishedTime - startTime, 'milliseconds')
    return finishedTime - startTime
  }


  const runBenchmarks = async () => {
    setIsProcessing(true)
    
    setNativeBase64SmallResult(handleNativeBase64(smallData))
    setTurbo64SmallResult(handleTurbo64Press(smallData))
    setNativeBase64BigResult(handleNativeBase64(bigData))
    setTurbo64BigResult(handleTurbo64Press(bigData))

    setIsProcessing(false)
  }


  const speedupSmall =
    turbo64SmallResult && nativeBase64SmallResult
      ? round(nativeBase64SmallResult / turbo64SmallResult) + 'x faster'
      : ' '

  const speedupBig =
    turbo64BigResult && nativeBase64BigResult
      ? round(nativeBase64BigResult / turbo64BigResult) + 'x faster'
      : ' '

  return (
    <View>
      <View style={{ flexDirection: 'row', alignSelf: 'flex-end', gap: 80 }}>
        <Text style={styles.heading}>5.9KB</Text>
        <Text style={styles.heading}>0.9MB</Text>
      </View>
      <View style={styles.lib}>
        <Text style={styles.heading}>react-native-quick-base64</Text>
        <Text style={styles.result}>
          {nativeBase64SmallResult > 0
            ? `${round(nativeBase64SmallResult, 2)} ms`
            : ''}
        </Text>
        <Text style={styles.result}>
          {nativeBase64BigResult > 0
            ? `${round(nativeBase64BigResult, 2)} ms`
            : ''}
        </Text>
      </View>

      <View style={styles.lib}>
        <Text style={styles.heading}>react-native-turbo-base64</Text>
        <Text style={styles.result}>
          {turbo64SmallResult > 0 ? `${round(turbo64SmallResult, 2)} ms` : ''}
        </Text>
        <Text style={styles.result}>
          {turbo64BigResult > 0 ? `${round(turbo64BigResult, 2)} ms` : ''}
        </Text>
      </View>

      <View style={{ flexDirection: 'row', alignSelf: 'flex-end', gap: 20 }}>
        <Text style={styles.speedup}>{speedupSmall}</Text>
        <Text style={styles.speedup}>{speedupBig}</Text>
      </View>
      <Pressable
        onPress={() => {
          runBenchmarks()
        }}
        style={styles.button}
      >
        <Text style={styles.pressable}>
          {isProcessing
            ? 'Processing...'
            : 'Run Benchmarks'}
        </Text>
      </Pressable>
    </View>
  )
}

export default Benchmarks

const styles = StyleSheet.create({
  box: {
    width: 60,
    height: 60,
    marginVertical: 20
  },
  lib: {
    flexDirection: 'row',
    justifyContent: 'space-between'
  },
  heading: {
    fontSize: 14,
    marginVertical: 5
  },
  pressable: {
    textAlign: 'center'
  },
  result: {
    fontFamily: Platform.OS === 'ios' ? 'Courier New' : 'monospace',
    flex: 1,
    textAlign: 'right',
    marginVertical: 5
  },
  button: { backgroundColor: 'skyblue', padding: 12, marginTop: 20 },
  speedup: {
    marginVertical: 5,
    fontSize: 18,
    textAlign: 'center'
  }
})
