FROM amd64/ubuntu:20.04 AS base

#TLS Wallet
LABEL maintainer="TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe"

EXPOSE 8766/tcp
EXPOSE 8767/tcp

ENV DEBIAN_FRONTEND=noninteractive

#Install runtime dependencies
RUN apt-get update && \
	apt-get install -y --no-install-recommends \
	bash wget net-tools libminiupnpc17 \
	libevent-2.1 libevent-pthreads-2.1 \
	libboost-system1.71 libboost-filesystem1.71 libboost-chrono1.71 \
	libboost-program-options1.71 libboost-thread1.71 \
	libzmq5 && \
	apt-get clean

FROM base AS build

#Install build dependencies
RUN apt-get update && \
	apt-get install -y --no-install-recommends \
	bash net-tools build-essential libtool autotools-dev automake \
	pkg-config libssl-dev libevent-dev bsdmainutils python3 \
	libboost-system1.71-dev libboost-filesystem1.71-dev libboost-chrono1.71-dev \
	libboost-program-options1.71-dev libboost-test1.71-dev libboost-thread1.71-dev \
	libzmq3-dev libminiupnpc-dev && \
	apt-get clean

#Copy source dir
COPY . /home/telestai/build/Telestai/
WORKDIR /home/telestai/build/Telestai

#build db4 from source
WORKDIR /home/telestai/build/Telestai/contrib
RUN ./install_db4.sh ../../

# build Telestaicore
WORKDIR /home/telestai/build/Telestai
RUN ./autogen.sh && ./configure --disable-tests BDB_LIBS="-L/home/telestai/build/db4/lib -ldb_cxx-4.8" BDB_CFLAGS="-I/home/telestai/build/db4/include" --with-gui=no  && make -j4

FROM base AS final

#Add our service account user
RUN useradd -ms /bin/bash telestai && \
	mkdir /var/lib/telestai && \
	chown telestai:telestai /var/lib/telestai && \
	ln -s /var/lib/telestai /home/telestai/.telestai && \
	chown -h telestai:telestai /home/telestai/.telestai

VOLUME /var/lib/telestai

#Copy the compiled binaries from the build
COPY --from=build /home/telestai/build/Telestai/src/telestaid /usr/local/bin/telestaid
COPY --from=build /home/telestai/build/Telestai/src/telestai-cli /usr/local/bin/telestai-cli

WORKDIR /home/telestai
USER telestai

CMD /usr/local/bin/telestaid -datadir=/var/lib/telestai -printtoconsole -onlynet=ipv4
