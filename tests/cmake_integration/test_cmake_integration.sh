#!/usr/bin/env bash
set -xeuo pipefail
cd "$( dirname "${BASH_SOURCE[0]}")"  # cd here

# All output should be within this directory.
TMP_DIR="${PWD}/tmp"

# Root of the cmake integration tests.
TEST_DIR="${PWD}"

# Path of the sources, build and install directory of HighFive.
HIGHFIVE_DIR="${TEST_DIR}/../.."
HIGHFIVE_BUILD_DIR="${TMP_DIR}/build-highfive"
HIGHFIVE_INSTALL_DIR="${HIGHFIVE_BUILD_DIR}/install"

export HIGHFIVE_GIT_REPOSITORY="file://$(realpath "$HIGHFIVE_DIR")"
export HIGHFIVE_GIT_TAG=$(git rev-parse HEAD)

make_submodule() {
    local project_dir="$1"
    local dep_dir="${project_dir}/deps/HighFive"

    rm "${dep_dir}" || true
    mkdir -p "$(dirname "${dep_dir}")"
    ln -sf "${HIGHFIVE_DIR}" "${dep_dir}"

    tar -C ${project_dir}/deps -cf ${project_dir}/deps/HighFive.tar HighFive
}

test_dependent_library() {
    local project="dependent_library"
    local project_dir="${TEST_DIR}/${project}"

    make_submodule ${project_dir}

    for use_boost in On Off
    do
      local build_dir="${TMP_DIR}/build"
      local install_dir="${TMP_DIR}/build/install"

      for lib_vendor in external_build submodule_excl submodule_incl fetch_content external_install
      do
        rm -rf ${build_dir} || true

        if [[ ${lib_vendor} == external_install ]]
        then
          cmake_extra_args=(
            -DCMAKE_PREFIX_PATH="${HIGHFIVE_INSTALL_DIR}"
            -DVENDOR_STRATEGY=external
          )
        elif [[ ${lib_vendor} == external_build ]]
        then
          cmake_extra_args=(
            -DCMAKE_PREFIX_PATH="${HIGHFIVE_BUILD_DIR}"
            -DVENDOR_STRATEGY=external
          )
        elif [[ ${lib_vendor} == submodule_excl ]]
        then
          cmake_extra_args=(
            -DCMAKE_PREFIX_PATH="${HIGHFIVE_BUILD_DIR}"
            -DVENDOR_STRATEGY=${lib_vendor}
          )
        else
          cmake_extra_args=(
            -DVENDOR_STRATEGY=${lib_vendor}
          )
        fi

        cmake "$@" \
              -DUSE_BOOST=${use_boost} \
              -DCMAKE_INSTALL_PREFIX="${install_dir}" \
              ${cmake_extra_args[@]} \
              -B "${build_dir}" "${project_dir}"

        cmake --build "${build_dir}" --parallel --verbose --target install

        dep_vendor_strats=(none fetch_content external submodule)
        for dep_vendor in ${dep_vendor_strats[@]}
        do
          local test_project="test_dependent_library"
          local test_build_dir="${TMP_DIR}/test_build"
          local test_install_dir="${TMP_DIR}/test_build/install"

          make_submodule ${test_project}

          rm -rf ${test_build_dir} || true

          if [[ ${lib_vendor} == external*
                || ${lib_vendor} == "submodule_excl"
                || ${dep_vendor} == "external"
                || ${dep_vendor} == "none" ]]
          then
            cmake_extra_args=(
              -DCMAKE_PREFIX_PATH="${HIGHFIVE_INSTALL_DIR};${install_dir}"
            )
          else
            cmake_extra_args=(
              -DCMAKE_PREFIX_PATH="${install_dir}"
            )
          fi

          cmake -DUSE_BOOST=${use_boost} \
                -DVENDOR_STRATEGY=${dep_vendor} \
                -DCMAKE_INSTALL_PREFIX="${test_install_dir}" \
                ${cmake_extra_args[@]} \
                -B "${test_build_dir}" "${test_project}"

          cmake --build "${test_build_dir}" --parallel --verbose
          ctest --test-dir "${test_build_dir}" --verbose
        done
      done
    done
}

test_application() {
    local project="application"
    local project_dir="${TEST_DIR}/${project}"

    make_submodule ${project_dir}

    for vendor in submodule fetch_content external
    do
      for use_boost in On Off
      do
        local build_dir="${TMP_DIR}/build"
        local install_dir="${TMP_DIR}/build/install"

        rm -rf ${build_dir} || true

        cmake "$@" \
              -DUSE_BOOST=${use_boost} \
              -DVENDOR_STRATEGY=${vendor} \
              -DCMAKE_PREFIX_PATH="${HIGHFIVE_INSTALL_DIR}" \
              -DCMAKE_INSTALL_PREFIX="${install_dir}" \
              -B "${build_dir}" "${project_dir}"

        cmake --build "${build_dir}" --verbose --parallel --target install
        ctest --test-dir "${build_dir}"
        "${install_dir}"/bin/Hi5Application
      done
    done
}

cmake -DHIGHFIVE_EXAMPLES=OFF \
      -DHIGHFIVE_UNIT_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX="${HIGHFIVE_INSTALL_DIR}" \
      -B "${HIGHFIVE_BUILD_DIR}" \
      "${HIGHFIVE_DIR}"

cmake --build "${HIGHFIVE_BUILD_DIR}" --parallel --target install

for integration in full Include short bailout
do
  test_dependent_library \
      -DINTEGRATION_STRATEGY=${integration}

  test_application \
      -DINTEGRATION_STRATEGY=${integration}
done
