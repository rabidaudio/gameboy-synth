# midi notes 0-127. midi note 0 = C-1. A4 is note 69 (nice)
note_names = (0..127).map { |n| %w(C C# D D# E F F# G G# A A# B)[n % 12] + ((n / 12)-1).to_s }
# For A4, n = 0 -> 440 * 2^(0/12) = 440 Hz.
# n = (octave * 12) + semitones[key] - ((4 * 12) + semitones['A'])
# 440.0 * (2.0 ** (n / 12.0))
note_pitches = (0..127).map { |n| 440.0 * 2 ** ((n-69)/12.0) }
# Hz = 131072/(2048-period)
# period = (-131072.0 / f)+2048
# where period is a signed 11bit integer (0 to 2042)
# all_frequencies = (0..2047).map { |p| 131072.0/(2048-p)}
# hz = 440*2^(n-12) -> hz/440=2^(n-12) -> log2(hz/440)-12=n
# because the lowest we can go is 64Hz, we'll start the lookup table at C2 (36)
start = 36
note_names = note_names[start..]
note_pitches = note_pitches[start..]
periods = note_pitches.map { |f| ((-131072.0 / f)+2048).round }
puts periods.each_with_index.map { |p, i| "#{p}, \/* #{i+start},#{note_names[i]} *\/" }.each_slice(12).map { |s| s.join(" ") }.join("\n")
