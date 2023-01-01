package main

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"

	"github.com/faiface/beep/wav"
	"github.com/mjibson/go-dsp/spectral"
	"github.com/montanaflynn/stats"
)

type Value uint8

func (v Value) S() int {
	return int(uint8(v) >> 4)
}

func (v Value) W() int {
	if uint8(v)&0x08 == 0 {
		return 0
	}
	return 1
}

func (v Value) R() int {
	return int(uint8(v) & 0x07)
}

// w,s,r
type Results struct {
	AvgPower       [2][16][8]float64
	RelativePeak   [2][16][8]float64
	RelativeMedian [2][16][8]float64
	Values         [2][16][8]float64
}

var results = Results{
	AvgPower:       [2][16][8]float64{},
	RelativePeak:   [2][16][8]float64{},
	RelativeMedian: [2][16][8]float64{},
	Values:         [2][16][8]float64{},
}

func analyze(v Value) error {
	f, err := os.Open(fmt.Sprintf("samples/%d.wav", uint8(v)))
	if err != nil {
		return err
	}
	defer f.Close()
	streamer, format, err := wav.Decode(f)
	if err != nil {
		return err
	}

	buffer := make([][2]float64, streamer.Len())
	streamer.Stream(buffer)

	mono := make([]float64, len(buffer))
	for i := 0; i < len(buffer); i++ {
		mono[i] = buffer[i][0]
	}
	pwr, _ := spectral.Pwelch(mono, float64(format.SampleRate), &spectral.PwelchOptions{
		NFFT: 256, // we care more about resolution in the X than in the Y
		// Window:    window.Rectangular,
		// Scale off gives us bigger numbers, which is better for precison (since we only
		// care about how they are relative to each other instead of absolute units)
		Scale_off: true,
	})

	avg, err := stats.Mean(pwr)
	if err != nil {
		return err
	}
	max, err := stats.Max(pwr)
	if err != nil {
		return err
	}
	median, err := stats.Median(pwr)
	if err != nil {
		return err
	}
	normmax := max / avg
	normmedian := median / avg

	results.AvgPower[v.W()][v.S()][v.R()] = avg
	results.RelativePeak[v.W()][v.S()][v.R()] = normmax
	results.RelativeMedian[v.W()][v.S()][v.R()] = normmedian
	results.Values[v.W()][v.S()][v.R()] = float64(uint8(v))
	return nil
}

func main() {
	for i := 0; i < 256; i++ {
		if err := analyze(Value(i)); err != nil {
			log.Fatal(err)
		}
	}

	outf, err := os.Create("results.csv")
	if err != nil {
		log.Fatal(err)
	}
	defer outf.Close()

	MustWrite := func(value string) {
		if _, err := outf.WriteString(value); err != nil {
			log.Fatal(err)
		}
	}

	WriteFloats := func(values []float64) {
		ss := make([]string, len(values))
		for i, v := range values {
			ss[i] = strconv.FormatFloat(v, 'g', -1, 64)
		}
		MustWrite(strings.Join(ss[:], ","))
	}

	MustWrite("Avg no-W,,,,,,,,Avg W\n")
	for i := 0; i < 16; i++ {
		WriteFloats(results.AvgPower[0][i][:])
		MustWrite(",")
		WriteFloats(results.AvgPower[1][i][:])
		MustWrite("\n")
	}
	MustWrite("\nRelPeak no-W,,,,,,,,RelPeak W\n")
	for i := 0; i < 16; i++ {
		WriteFloats(results.RelativePeak[0][i][:])
		MustWrite(",")
		WriteFloats(results.RelativePeak[1][i][:])
		MustWrite("\n")
	}
	MustWrite("\nRelMedian no-W,,,,,,,,RelMedian W\n")
	for i := 0; i < 16; i++ {
		WriteFloats(results.RelativeMedian[0][i][:])
		MustWrite(",")
		WriteFloats(results.RelativeMedian[1][i][:])
		MustWrite("\n")
	}

	MustWrite("\nWaveform IDs\n")
	for i := 0; i < 16; i++ {
		WriteFloats(results.Values[0][i][:])
		MustWrite(",")
		WriteFloats(results.Values[1][i][:])
		MustWrite("\n")
	}
}
