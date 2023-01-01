require 'fileutils'
FileUtils.mkdir_p("samples")
(0...256).each do |i|
  puts i
  `./GameBoySynth #{i}`
  FileUtils.mv("out.wav", "samples/#{i}.wav")
end
