export SGX_SDK=/home/aghia/sgx_workspace/sgxsdk
export PATH=$PATH:$SGX_SDK/bin:$SGX_SDK/bin/x64
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$SGX_SDK/pkgconfig:~/usr/local/lib/pkgconfig
if [ -z "$LD_LIBRARY_PATH" ]; then
     export LD_LIBRARY_PATH=$SGX_SDK/sdk_libs
else
     export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SGX_SDK/sdk_libs
fi
