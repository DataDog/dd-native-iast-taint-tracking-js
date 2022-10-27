ARG NODE_VERSION
FROM node:$NODE_VERSION-alpine

RUN apk add py3-pip make g++ cmake cpputest cpputest-static

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["build"]
