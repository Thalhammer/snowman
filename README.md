# Snowman Hotword Detection

Snowman Hotword Detection is an open source rewrite of the popular
[Snowboy](https://github.com/Kitt-AI/snowboy/) library originally developed by Kitt.AI.
It was created in the hope of preserving support for and improving it, as well as allowing
it to be used on modern devices (embedded and desktop).

### Disclamer

While I did my best, reversing software tends to be more art than science, so it is
very likely that I introduced some bugs while doing so. The fact that I have very little
experience with audio processing and neural networks does not really help either. If you have
either of those and could spare some time proofreading what I did, that would be highly
appreciated. I won't provide any warranty for it, but I did my best to make sure this library
won't light your PC on fire.

### Feature support

In the default build configuration, it should be a drop-in replacement for the original snowboy
library. However, it does not implement everything the original library did. The most important
differences are the following:

- **Missing frontend processing**:
  I do not implement any automatic gain control or noise suppression (both were part of the
  library if you enabled "ApplyFrontend"), so make sure you have a good audio source until it
  is implemented. Voice Activity Detection (VAD) does work, however.

- **Missing support for some hotword search algorithms**:
  There are multiple hotword search algorithms used by universal models. I have only implemented
  "Naive" so far and added asserts to those that are completely unused and redirected used ones to
  the Naive method, which seems to work fine. However, we should probably implement all of them at
  some point.

- **Split Radix FFT**:
  There were two supported FFT modes, normal FFT and split radix FFT. So far I have only implemented
  normal FFT and hardcoded SRFFT models to use normal FFT. From my understanding, the result should
  be identical, but split radix FFT might have better performance.

- **PipelineVAD**:
  While reversed, it is totally untested. That said, most of the code is identical with PipelineDetect
  and thus somewhat tested, so I don't expect any major bugs in it.

- **Wave reading, PipelineNNETForward**:
  While present in the executable, they where never exposed with headers so no user code should
  rely on them. I might implement them at some point, though.


### Universal models

Existing universal models should work out of the box and perform similarly to the original library.
Since they are designed to work with "ApplyFrontend" disabled, the missing AGC/NoiseSuppression
should not have an effect.

New universal models should be doable in theory. However, I don't know enough about neural networks
to do so. If you do, **please** reach out to me. Another issue is the lack of a way to gather samples.
In the future I might build a website similar to the original kitt.ai website where people can train
their personal models using a nice UI, as well as an option to share audio samples for building
universal models, but this is still in the far future.

### Personal models
Training personal models is now possible using the `enroll` utility build along the library. While the resulting model is not bit identical with models trained using the original library, it is identical to 5 digits of precision. The remaining differences are most likely a result of rounding errors within the process and should not affect the performance of the model.

### Usage
As before the main interface is `snowboy-detect.h` which includes the well known `snowboy::SnowboyDetect`, `snowboy::SnowboyVad`, `snowboy::SnowboyPersonalEnroll` and `snowboy::SnowboyTemplateCut` classes. Those classes provide a very high level interface to snowboy that should be sufficient for most applications. There is also a file `snowboy-detect-c.h` file which provides a C wrapper for the beforementioned classes and should make integration into other languages a lot easier.

### Contributing

Any help would be highly appreciated. I am particularly looking for people with knowledge of machine
learning, audio processing or reverse engineering. However, all help is welcome. Simply take a look
at the open issues and pull requests, as well as the [TODO.md](TODO.md) file.

### License

The original project was licensed under the Apache 2 license, so this one is as well.
Just please don't use it as a voice interface for Skynet.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this software except in compliance with the License.
You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0) or
[here](LICENSE.txt)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
