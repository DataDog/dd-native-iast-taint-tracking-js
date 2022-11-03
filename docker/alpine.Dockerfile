ARG NODE_VERSION
FROM node:$NODE_VERSION-alpine

RUN apk add py3-pip make g++ cmake cpputest 
# node 13 image does not have cpputest-static as a separate packege
# the statil library is included in cpputest package. However for the
# the rest of the versions cpputest-static is needed.
RUN apk add cpputest-static || true

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["build"]
