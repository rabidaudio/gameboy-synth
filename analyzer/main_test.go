package main_test

import (
	"fmt"
	"io/ioutil"
	"os"
	"testing"
	"time"

	"github.com/faiface/beep/wav"
	"github.com/mjibson/go-dsp/spectral"
	"gotest.tools/v3/assert"
)

func TestFFT(t *testing.T) {
	f, err := os.Open("test.wav")
	assert.NilError(t, err)
	streamer, format, err := wav.Decode(f)
	assert.NilError(t, err)

	assert.Equal(t, format.NumChannels, 2)
	assert.Equal(t, format.SampleRate.N(time.Second), 44100)
	assert.Assert(t, streamer.Len() >= 32768)

	buffer := make([][2]float64, 32768)
	n, ok := streamer.Stream(buffer)
	assert.Assert(t, ok)
	assert.Assert(t, n == 32768)

	mono := make([]float64, 32768)
	for i := 0; i < 32768; i++ {
		mono[i] = buffer[i][0]
	}
	pwr, freq := spectral.Pwelch(mono, float64(format.SampleRate), &spectral.PwelchOptions{
		NFFT: 256, // we care more about resolution in the X than in the Y
		// Window:    window.Rectangular,
		// Scale off gives us bigger numbers, which is better for precison (since we only
		// care about how they are relative to each other instead of absolute units)
		Scale_off: true,
	})
	s := ""
	for i := 0; i < len(pwr); i++ {
		s += fmt.Sprintf("%f\t%f\n", freq[i], pwr[i])
	}
	ioutil.WriteFile("stats.txt", []byte(s), 0)
}
