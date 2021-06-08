let Snowman;
let detector;
let _bufferAddr;
let _bufferSize;
const resultsContainer = document.getElementById("detection-result");
const logContainer = document.getElementById("detection-log");

async function load(resUrl, modelUrl) {
    const storagePath = "/snowman";
    const resPath = storagePath + "/" + resUrl.replace(/[\W]/g, "_");
    const modelPath = storagePath + "/" + modelUrl.replace(/[\W]/g, "_");
    return new Promise((resolve, reject) =>
            LoadSnowman().then((loaded) => {
                Snowman = loaded;
                resolve(true);
            })
        )
        .then(() => {
            console.debug("Setting up persistent storage at " + storagePath);
            Snowman.FS.mkdir(storagePath);
            Snowman.FS.mount(Snowman.IDBFS, {}, storagePath);
            return Snowman.syncFilesystem(true);
        })
        .then(() => {
            const fullModelUrl = new URL(
                modelUrl,
                location.href.replace(/^blob:/, "")
            );
            console.debug(`Downloading ${fullModelUrl} to ${modelPath}`);
            const fullResUrl = new URL(
                resUrl,
                location.href.replace(/^blob:/, "")
            );
            console.debug(`Downloading ${fullResUrl} to ${resPath}`);
            return Promise.all([Snowman.download(fullModelUrl, modelPath), Snowman.download(fullResUrl, resPath)]);
        })
        .then(() => {
            console.debug(`Syncing filesystem`);
            return Snowman.syncFilesystem(false);
        })
        .then(() => {
            console.debug(`Creating detector`);
            detector = new Snowman.SnowboyDetect(resPath, modelPath);
            console.log(
                `Detector created! sampleRate: ${detector.SampleRate()} numChannels: ${detector.NumChannels()} bitsPerSample: ${detector.BitsPerSample()}`
            );
            console.log(
                `Detector created! sensitivity: ${detector.GetSensitivity()} numHotwords: ${detector.NumHotwords()}`
            );
            detector.ApplyFrontend(false);
        })
        .then(() => {
            postMessage({
                action: "load",
                result: true
            });
            return true;
        });
}

load("common.res", "snowboy.umdl").then(() => {
    resultsContainer.textContent = "Ready";
});

function _allocateBuffer(size) {
    if (_bufferAddr !== null && _bufferSize === size) {
        return;
    }
    _freeBuffer();
    _bufferAddr = Snowman._malloc(size);
    console.debug(
        `DetectionWorker: allocated buffer with address ${_bufferAddr}`
    );
    _bufferSize = size;
    console.debug(
        `DetectionWorker: allocated buffer of ${_bufferSize} bytes`
    );
}

function _freeBuffer() {
    if (_bufferAddr === null) {
        return;
    }
    Snowman._free(_bufferAddr);
    console.debug(`DetectionWorker: freed buffer of ${_bufferSize} bytes`);
    _bufferAddr = null;
    _bufferSize = null;
}

function processAudioChunk(data) {
    const requiredSize = data.length * data.BYTES_PER_ELEMENT;
    _allocateBuffer(requiredSize);
    if(data instanceof Int16Array) {
        Snowman.HEAP16.set(data, _bufferAddr / data.BYTES_PER_ELEMENT);
        return detector.RunDetectionI16(_bufferAddr, data.length, false);
    } else if(data instanceof Int32Array) {
        Snowman.HEAP32.set(data, _bufferAddr / data.BYTES_PER_ELEMENT);
        return detector.RunDetectionI32(_bufferAddr, data.length, false);
    } else if(data instanceof Float32Array) {
        Snowman.HEAPF32.set(data, _bufferAddr / data.BYTES_PER_ELEMENT);
        return detector.RunDetectionF32(_bufferAddr, data.length, false);
    } else throw new Error("Invalid datatype passed to processAudioChunk");
}

async function init() {
    const sampleRate = 16000;
    const mediaStream = await navigator.mediaDevices.getUserMedia({
        video: false,
        audio: {
            echoCancellation: true,
            noiseSuppression: true,
            channelCount: 1,
            sampleRate,
            sampleSize: 16,
        },
    });

    const audioContext = new AudioContext({
        sampleRate
    });
    await audioContext.audioWorklet.addModule("audio-processor.js");
    audioProcessor = new AudioWorkletNode(audioContext, "audio-processor", {
        channelCount: 1,
        numberOfInputs: 1,
        numberOfOutputs: 1,
    });

    audioProcessor.port.onmessage = (ev) => {
        const result = processAudioChunk(ev.data.buffer);
        resultsContainer.textContent = result;
        if (result > 0)
            logContainer.innerHTML += new Date().toLocaleTimeString() + " detected hotword " + result + "<br>"
    };

    source = audioContext.createMediaStreamSource(mediaStream);
    resultsContainer.textContent = "Ready";

    const trigger = document.getElementById("trigger");
    const stopper = document.getElementById("stopper");

    trigger.onmouseup = () => {
        trigger.disabled = true;
        stopper.disabled = false;
        if (!source) {
            init().then(() => source.connect(audioProcessor));
        } else {
            source.connect(audioProcessor);
        }
    };

    stopper.onmouseup = () => {
        trigger.disabled = false;
        stopper.disabled = true;
        source.disconnect(audioProcessor);
    };
}

window.onload = () => {
    init();
};