# oglFusionKinectV2
An openGL GLSL implementation of KinectFusion for the Kinect v2

![oglFusion](docs/oglfusionNM.jpg?raw=true "near mode Kinect v2")


<h2>Installation</h2>

<h3>Dependencies</h2>

We use vcpkg to install dependencies. Get vcpkg from the link and follow its installation instructions.

<a href="https://github.com/Microsoft/vcpkg">VCPKG</a> 

<h4>Windows</h3>

To make vcpkg use a little bit cleaner we set two environment variables, defining the tpe of system (x64 / x86) and the location of vcpkg.exe. Open a command promt with administrator privilages (hit windows key, type "cmd", right click "Command Prompt" and choose "Run as Administrator") .
These commands may take a few seconds to execute.

```
setx VCPKG_DEFAULT_TRIPLET "x64-windows" /m
setx VCPKG_DIR "C:\vcpkg" /m
```
Close the Admin Command Prompt window to flush the newly set variables.

Go to your vcpkg.exe installed location and open another command prompt.

Then we install the various libraries needed for this project.

```
vcpkg install glew glfw3 glm imgui eigen3 tinyxml2 nlohmann-json
```
This should take 2-3 minutes.

The libfreenect2 version in vcpkg is old, and needs updating. Also it doesnt have simple support for near mode. So we use our forked version of <a href="https://github.com/philipNoonan/libfreenect2">Libfreenect2</a>.

To install this via vcpkg, copy the libfreenect2-nm folder in "./depends/" to your "./vcpkg/ports/" directory. 

Also, copy ands replace the libusb folder in "./depends/" to your "./vcpkg/ports/" directory.

Then you can install via vcpkg

```
vcpkg install libusb libfreenect2-nm
```

<h3> Using Zadig </h3>

<p>Install the libusbK backend driver for libusb. Please follow the steps exactly:</p>
<ol>
<li>Download Zadig from <a href="http://zadig.akeo.ie/" rel="nofollow">http://zadig.akeo.ie/</a>.</li>
<li>Run Zadig and in options, check "List All Devices" and uncheck "Ignore Hubs or Composite Parents"</li>
<li>Select the "Xbox NUI Sensor (composite parent)" from the drop-down box. (Important: Ignore the "NuiSensor Adaptor" varieties, which are the adapter, NOT the Kinect) The current driver will list usbccgp. USB ID is VID 045E, PID 02C4 or 02D8.</li>
<li>Select libusbK (v3.0.7.0 or newer) from the replacement driver list.</li>
<li>Click the "Replace Driver" button. Click yes on the warning about replacing a system driver. (This is because it is a composite parent.)</li>
</ol>
<p>To uninstall the libusbK driver (and get back the official SDK driver, if installed):</p>
<ol>
<li>Open "Device Manager"</li>
<li>Under "libusbK USB Devices" tree, right click the "Xbox NUI Sensor (Composite Parent)" device and select uninstall.</li>
<li>Important: Check the "Delete the driver software for this device." checkbox, then click OK.</li>
</ol>

<h3> Installing oglFusion </h3>

We use <a href="https://www.visualstudio.com/downloads/">visual studio 2017</a> since it is the most readily available MSVC these days, support for c++17 features, and the hope that it will be useable with cuda 9.2.

We use <a href="https://cmake.org/download/">cmake</a> . Please use the latest version available.

Pull the latest version of oglFusion

```
git clone https://github.com/philipNoonan/oglFusionKinectV2
```

Open CMake and set the source directory as "PATH_TO_YOUR_VERSION/oglFusionKinectV2/" and the build directory as "PATH_TO_YOUR_VERSION/oglFusionKinectV2/build"

Choose to create a new folder, and choose MSVC 15 2017 x64 as the generator.

Press "Configure"

Press "Generate"

Press "Open Project"

Inside Visual Studio, you can build/run the solution. After the app window opens, press "Load Calib" and then "Start Kinect" 













