ARG TAG
FROM debian:$TAG

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

RUN apt-get update && apt-get -y upgrade && apt-get -y install \
    make \
    cmake \
    g++ \
    cpputest \
    libcpputest-dev

COPY . .
CMD ["sh", "./scripts/cpputest.sh", "--ci"]
