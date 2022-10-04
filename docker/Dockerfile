ARG NODE_VERSION
FROM node:$NODE_VERSION

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

RUN apt-get update && apt-get -y install \
    make \
    cmake \
    g++ \
    cpputest \
    libcpputest-dev

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["build"]

