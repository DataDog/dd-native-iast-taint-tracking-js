ARG NODE_VERSION
FROM node:$NODE_VERSION

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

RUN if [ "${NODE_ARG_VERSION}" = "15" ] ; then \
        curl https://pyenv.run | bash ; \
        echo 'export PYENV_ROOT="$HOME/.pyenv"' >> /root/.bashrc ; \
        echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> /root/.bashrc ; \
        echo 'eval "$(pyenv init -)"' >> /root/.bashrc ; \
        export PYENV_ROOT="$HOME/.pyenv" ; \
        command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH" ; \
        eval "$(pyenv init -)" ; \
        pyenv install -v 3.6.15 ; \
        pyenv global 3.6.15 ; \
    fi

RUN npm ci --ignore-scripts
COPY . .
ENTRYPOINT ["npm", "run"]
CMD ["build"]

