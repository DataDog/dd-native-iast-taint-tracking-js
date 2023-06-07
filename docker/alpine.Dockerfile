ARG TAG
FROM alpine:$TAG

RUN apk add py3-pip make g++ cmake cpputest 
# node 13 image does not have cpputest-static as a separate packege
# the statil library is included in cpputest package. However for the
# the rest of the versions cpputest-static is needed.
RUN apk add cpputest-static || true

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

COPY . .
CMD ["sh", "./scripts/cpputest.sh", "--ci"]
