ARG NODE_VERSION
ARG NODE_IMAGE
FROM node:$NODE_IMAGE

ENV NODE_ARG_VERSION=${NODE_VERSION}
WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

RUN apt-get update && apt-get -y upgrade && apt-get -y install \
    make \
    cmake \
    g++ \
    cpputest \
    libcpputest-dev

RUN if [ "$(echo $NODE_VERSION | cut -d'.' -f1)" = "15" ] ; then \
        apt-get -y install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev curl libbz2-dev ; \
        curl -O https://www.python.org/ftp/python/3.7.3/Python-3.7.3.tar.xz ; \
        tar -xf Python-3.7.3.tar.xz ; \
        cd Python-3.7.3 ; \
        ./configure ; \
        make -j $(nproc) ; \
        make install ; \
    fi

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["build"]

