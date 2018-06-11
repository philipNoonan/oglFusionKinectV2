include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO philipNoonan/libfreenect2
    REF 0ee1ee2f8d2331253e78aaac0a340436b14ba34e
    SHA512 37916076693cbe4bad5c5a17f9c77e1836f0677bd54e9f7aa68c16e561394c94f2f3353a86e8aa52d632326ca60d93c9a06b236fc70047873dbff5d68077eb2d
    HEAD_REF master
)

file(READ ${SOURCE_PATH}/cmake_modules/FindLibUSB.cmake FINDLIBUSB)
string(REPLACE "(WIN32)"
               "(WIN32_DISABLE)" FINDLIBUSB "${FINDLIBUSB}")
file(WRITE ${SOURCE_PATH}/cmake_modules/FindLibUSB.cmake "${FINDLIBUSB}")

file(READ ${SOURCE_PATH}/examples/CMakeLists.txt EXAMPLECMAKE)
string(REPLACE "(WIN32)"
               "(WIN32_DISABLE)" EXAMPLECMAKE "${EXAMPLECMAKE}")
file(WRITE ${SOURCE_PATH}/examples/CMakeLists.txt "${EXAMPLECMAKE}")

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DENABLE_CUDA=OFF
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH "lib/cmake/freenect2")

vcpkg_copy_pdbs()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

# The cmake config is actually called freenect2Config.cmake instead of libfreenect2Config.cmake ...
file(RENAME ${CURRENT_PACKAGES_DIR}/share/libfreenect2-nm ${CURRENT_PACKAGES_DIR}/share/freenect2)

# license file needs to be in share/libfreenect2 otherwise vcpkg will complain
file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/share/libfreenect2-nm/)
file(COPY ${SOURCE_PATH}/GPL2 DESTINATION ${CURRENT_PACKAGES_DIR}/share/libfreenect2-nm/)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/libfreenect2-nm/GPL2 ${CURRENT_PACKAGES_DIR}/share/libfreenect2-nm/copyright)
