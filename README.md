# stm32f1-usb-gofast
How fast can USBFS peripheral go on the STM32F1?

## Building
```console
$ mkdir stm32f1-usb-gofast-workspace
$ cd
$ git clone https://github.com/jevinskie/stm32f1-usb-gofast
$ west init -l stm32-usb-gofast
$ west update
$ # TODO: cmake + ninja build
```

Or maybe... (untested)
```console
$ west init -m https://github.com/jevinskie/stm32f1-usb-gofast stm32f1-usb-gofast-workspace
$ west update
$ # TODO: cmake + ninja build
```

[West Manifests](https://docs.zephyrproject.org/latest/develop/west/manifest.html)
