FROM dockcross/web-wasm:20200119-1c10fb2

RUN apt-get update \
    && apt-get install -y \
    gfortran \
    libc6-dev-i386

WORKDIR /opt/kaldi
RUN git clone -b lookahead --single-branch https://github.com/alphacep/kaldi . && \
    git checkout 89bc31d

WORKDIR /opt/kaldi/tools/CLAPACK-WA
RUN git clone https://github.com/adrianbg/CLAPACK-WA .
ADD wasm/builder/CLAPACK-WA.patch /tmp
RUN git apply -v /tmp/CLAPACK-WA.patch
RUN emmake make -j $(nproc)

WORKDIR /opt/kaldi/tools/gsl
RUN curl --fail -q -L http://mirrors.ocf.berkeley.edu/gnu/gsl/gsl-2.6.tar.gz | \
    tar xz --strip-components=1
RUN emconfigure ./configure
RUN emmake make -j $(nproc)

WORKDIR /snowman_build
RUN echo "IyEvYmluL2Jhc2gKY21ha2UgL3Nub3dtYW4gLURDTUFLRV9CVUlMRF9UWVBFPVJlbGVhc2UgLURTTk9XTUFOX0JVSUxEX1RFU1RTPU9GRiBcCiAgICAgICAgLURTTk9XTUFOX0JVSUxEX0FQUFM9T0ZGIC1EU05PV01BTl9CVUlMRF9CRU5DSE1BUktTPU9GRiBcCiAgICAgICAgLURTTk9XTUFOX0JVSUxEX1dBU009T04gLURTTk9XTUFOX0NYWDExX0NPTVBBVD1PRkYgLURTTk9XTUFOX0VOQUJMRV9MVE89T0ZGCm1ha2UgLWokKG5wcm9jKSBzbm93Ym95X3dhc20KY3Agd2FzbS9zbm93Ym95X3dhc20ue2pzLGJjfSAvc25vd21hbl9vdXRwdXQv" | base64 -d > run.sh
RUN chmod +x run.sh
CMD [ "/snowman_build/run.sh" ]

# Build it
# docker build -t snowman_wasm_builder -f wasm/builder/Dockerfile .
# Run it
# docker run --rm -v $(pwd):/snowman -v$(pwd)/wasm_output:/snowman_output snowman_wasm_builder
# Afterwards snowboy_wasm.js and snowboy_wasm.bc are placed in the output folder.