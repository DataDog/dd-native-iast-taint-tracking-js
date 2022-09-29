FROM node:16-alpine@sha256:38bc06c682ae1f89f4c06a5f40f7a07ae438ca437a2a04cf773e66960b2d75bc

RUN apk add py3-pip make g++ cmake cpputest cpputest-static

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .
COPY ./scripts /build/scripts

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["pack-all"]
