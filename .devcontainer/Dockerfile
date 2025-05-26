FROM registry.gitlab.com/kothardev/kothar-script/devcontainer

USER root

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    # VOL REST requirements
    libcurl4-openssl-dev \
    libyajl-dev \
    zlib1g-dev \
    # pyenv requirement
    make build-essential libssl-dev zlib1g-dev \
    libbz2-dev libreadline-dev libsqlite3-dev curl git \
    libncursesw5-dev xz-utils tk-dev libxml2-dev \
    libxmlsec1-dev libffi-dev liblzma-dev ca-certificates \
    # tools
    telnet \
    && rm -rf /var/lib/apt/lists/*

# # Build REST VOL
# # https://github.com/HDFGroup/vol-rest
WORKDIR /HDFGroup
RUN git clone https://github.com/HDFGroup/vol-rest
WORKDIR /HDFGroup/vol-rest
RUN git checkout ${REST_VOL_COMMIT}
RUN mkdir build && cd build && \
    cmake -DHDF5_VOL_REST_ENABLE_CURL_DEBUG=ON -DHDF5_VOL_REST_ENABLE_DEBUG=ON -DCMAKE_INSTALL_PREFIX=/usr/local/vol-rest -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DHDF5_VOL_REST_ENABLE_EXAMPLES=OFF .. && \
    cmake --build . && cmake --install .

USER kothar
# We need pyenv to use older version of python for HSDS
ENV PYENV_ROOT="/home/kothar/.pyenv"
ENV PATH="${PYENV_ROOT}/bin:${PYENV_ROOT}/shims:${PATH}"
RUN curl https://pyenv.run | bash
RUN pyenv install 3.9.21 && pyenv global 3.9.21
RUN pip install hsds h5py h5pyd --break-system-packages

# Setup env variables for HSDS / REST VOL
ENV HDF5_PLUGIN_PATH=/usr/local/vol-rest/lib
ENV HDF5_VOL_CONNECTOR=REST
ENV HSDS_USERNAME=admin
ENV HSDS_PASSWORD=admin
ENV HSDS_ENDPOINT=http://localhost:5101

# The following is for h5pyd
ENV HS_USERNAME=admin
ENV HS_PASSWORD=admin
ENV HS_ENDPOINT=http://localhost:5101


# Create `~/launch-hsds.sh script file
RUN mkdir -p /home/kothar/.bin && cat <<'EOF' > /home/kothar/.bin/start-hsds
#!/bin/bash
mkdir -p /home/kothar/hsds_data
hsds --root_dir /home/kothar/hsds_data --host localhost --hs_username ${HSDS_USERNAME} --hs_password ${HSDS_PASSWORD} --count 2 &
EOF
RUN chmod +x /home/kothar/.bin/start-hsds
ENV PATH="/home/kothar/.bin:${PATH}"
RUN echo 'echo -e "\n🌟 Welcome to the Highfive Dev Container!\n📦 The REST VOL and HSDS server have been installed / configured.\n🚀 Use start-hsds to start the server...\n"' >> ~/.bashrc
