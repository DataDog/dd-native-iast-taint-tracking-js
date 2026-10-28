ARG TAG
FROM alpine:$TAG

# The 3.8 and 3.18 compatibility builds cannot receive a fixed CA package.
# Remove both roots from the source files and regenerate the system bundle.
RUN apk add ca-certificates && sed -i -e 's|^mozilla/emSign_ECC_Root_CA_-_G3.crt|!&|' -e 's|^mozilla/Comodo_AAA_Services_root.crt|!&|' /etc/ca-certificates.conf \
    && rm -f /usr/share/ca-certificates/mozilla/emSign_ECC_Root_CA_-_G3.crt /usr/share/ca-certificates/mozilla/Comodo_AAA_Services_root.crt \
    && update-ca-certificates && ln -sf /etc/ssl/certs/ca-certificates.crt /etc/ssl/cert.pem

RUN apk add py3-pip make g++ cmake cpputest 
# The EOL Python releases cannot use a patched pip; use the filtered system bundle.
RUN for bundle in /usr/lib/python*/site-packages/pip/_vendor/certifi/cacert.pem; do \
        if [ -f "$bundle" ]; then ln -sf /etc/ssl/certs/ca-certificates.crt "$bundle"; fi; \
    done
# node 13 image does not have cpputest-static as a separate packege
# the statil library is included in cpputest package. However for the
# the rest of the versions cpputest-static is needed.
RUN apk add cpputest-static || true

WORKDIR /build
COPY ./package.json .
COPY ./package-lock.json .

COPY . .
CMD ["sh", "./scripts/cpputest.sh", "--ci"]
