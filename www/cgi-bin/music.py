#!/usr/bin/env python3

import io
import os
import sys
import math
import wave
import struct

SAMPLE_RATE = 44100
DURATION = 5
STEP_MS = 50

steps = int(DURATION * 1000 / STEP_MS)

with open("/dev/urandom", "rb") as f:
	random_data = f.read(steps)

wav_buffer = io.BytesIO()

with wave.open(wav_buffer, "wb") as wav:
	wav.setnchannels(1)
	wav.setsampwidth(2)
	wav.setframerate(SAMPLE_RATE)

	frames = bytearray()

	for rnd in random_data:
		freq = 100 + rnd * 1400 / 255

		samples_this_step = SAMPLE_RATE * STEP_MS // 1000

		for i in range(samples_this_step):
			t = i / SAMPLE_RATE

			sample = int(
				12000 * math.sin(2 * math.pi * freq * t)
			)

			frames.extend(struct.pack("<h", sample))

	wav.writeframes(frames)

wav_data = wav_buffer.getvalue()

sys.stdout.buffer.write(
	(
		"Content-Type: audio/wav\r\n"
		f"Content-Length: {len(wav_data)}\r\n"
		"\r\n"
	).encode("ascii")
)

sys.stdout.buffer.write(wav_data)
sys.stdout.buffer.flush()
