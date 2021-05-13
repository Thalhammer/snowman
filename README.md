# Snowman Hotword Detection
Snowman Hotword Detection is an open source rewrite of the popular snowboy library 
originaly developed by Kitt.AI. It was created in the hope of preserving support for it,
improving it as well as allowing it to be used on modern devices (embedded and desktop).

It is based on the latest publically released version of snowboy which was built on Feb. 6th 2018.

While planned, this repository does not yet include the code required to train custom hotwords. Checkout the [official repo](https://github.com/seasalt-ai/snowboy) for it.

### Disclamer
While I did my best, reversing software tends to be more of an art than a science, so it is very likely that I introduced some bugs while doing so. The fact that I have very little experience with audio processing and neural networks does not really help either. If you have either of those and could spend you some time to proof read what I did it would be highly appreciated. I wont provide any warranty for it, but I did my best to make sure this library won't light your PC on fire.

### License
The original was licensed under Apache, so this is as well.
Please just dont use as a voice interface for Skynet.

### Feature support
In the default build configuration it should be a drop in replacement for the original snowboy library, however it does not implement everything the original library did. The most important differences are:#

- **Missing frontend processing.**\
  I do not implement any automatic gain control or noise suppression (both where part of the library if you enabled "ApplyFrontend"), so make sure you have a good audio source until it is implemented. Voice Activity Detection (VAD) does work however.
- **Missing support for some hotword search algorithms**\
  There are multiple hotword search algorithms used by universal models. I have only implement "Naive" so far and added asserts to those that are completely unused and redirect used ones to the Naive method which seems to work fine. However we should probably implement all of them at some point.
- **Split Radix FFT**\
  There were two supported fft modes, normal fft and split radix fft. I only implemented normal FFT yet and hardcoded srfft models to normal fft. From my understanding the result should be identical, but split radix fft might have better performance.
- **PipelineVAD**\
  While reversed, it is totally untested. That said most of the code is identical with PipelineDetect and thus somewhat tested, so I dont expect any major bugs in it.
- **Wave reading, PipelineNNETForward**\
  While present in the executable, they where never exposed with headers so no user code should rely on them. I might implement them at some point though.
- **Personal model training**\
  With the takeover by seasalt.ai a library containing code to train personal models got released as well. I have not reversed it yet, but I took a quick look at it and it uses most of the same backend code, so it should be fairly simple to implement. However I need to setup a test environment for it first, so give it some time. Until then you can train models using the official docker container.

### Universal models
Existing universal models should work out of the box and perform similar to the original library. Since they are designed to work with "ApplyFrontend" dissabled the missing AGC/NoiseSuppression should not affect those.

New universal models should be doable in theory, however I don't know enough about neural networks to do so. If you do, **please** reach out to me. Another issue is the lack of way to gather samples. In the future I might build a website similar to the original kitt.ai website where people can train there personal models using a nice ui, as well as optin to share audio samples for building universal models, but this is still in the far future.

### Contributing
If you want to help, it is highly appreciated. I am specifically looking for people with knowledge about Machine Learning, Audio Processing or Reverse engeneering, however every help is welcome (Yes, even the grammar/spelling error you found in this file!). Simply take a look at the open issues, pull requests as well as the [TODO.md](./TODO.md) file.