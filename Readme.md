<br>
<p align="center">
  <img width=384 src="https://download.nap-labs.tech/identity/svg/logos/nap_logo_blue.svg">
</p>
	
# Description

Calculates the sunset & sunrise based on longitude, latitude and time using the [Sunset](https://github.com/buelowp/sunset) library in Nap.

## Installation
Compatible with NAP 0.7 and higher - [package release](https://github.com/napframework/nap/releases) and [source](https://github.com/napframework/nap) context. 

### From ZIP

[Download](https://github.com/naivisoftware/napsunset/archive/refs/heads/main.zip) the module as .zip archive and install it into the nap `modules` directory:
```
cd tools
./install_module.sh ~/Downloads/napsunset-main.zip
```

### From Repository

Clone the repository and setup the module in the nap `modules` directory.

```
cd modules
clone https://github.com/naivisoftware/napsunset.git
./../tools/setup_module.sh napsunset
```

## Demo

Includes a simple demo that shows if the sun is up or down based on the provided settings of the `nap::SunsetCalculatorComponent`
