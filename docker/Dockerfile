FROM node:16@sha256:481d28c3890e832ec54b73ec5ea74d91b91af111ec06ae3a3bcb5a81e91892f0

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .
COPY ./scripts/ /build/scripts/

RUN apt-get update && apt-get -y install \
    make \
    cmake \
    g++ \
    cpputest \
    libcpputest-dev

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["pack-all"]

