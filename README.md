# Smart_gym


<!--
*** Thanks for checking out this README Template. If you have a suggestion that would
*** make this better, please fork the repo and create a pull request or simply open
*** an issue with the tag "enhancement".
*** Thanks again! Now go create something AMAZING! :D
-->





<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![LinkedIn][linkedin-shield]][linkedin-url]



<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/KristianDukov/Smart_gym">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">Smart_Gym</h3>

  <p align="center">
    How to use Arduino to create your own smart gym!
  </p>
</p>



<!-- TABLE OF CONTENTS -->
## Table of Contents

* [About the Project](#about-the-project)
  * [Built With](#built-with)
* [Getting Started](#getting-started)
  * [Prerequisites](#prerequisites)
  * [Installation](#installation)
* [Usage](#usage)
* [Roadmap](#roadmap)
* [Contributing](#contributing)
* [Contact](#contact)



<!-- ABOUT THE PROJECT -->
## About The Project

[![Product Name Screen Shot][product-screenshot]](https://example.com)

The idea of this project is to create a device that will be able to track the way how are you performing physical exercises using fitness machines.

Here is how it work:
* Using an IR sensor the microcontroller track the distance of the weight plates located on the machine which are moving up and down when making the exercise
* When a certain threshold is reached (for each person and machine it can be different; refer to the calibration) the time is tracked and the number of rep is sent via websocket to the clients connected to the microcontroller. Arduino is hosting multiple services such as the dashboard wher you can track your progress on any device (HTML,CSS and JS) and the websocket server responsible for sending of the data
* When the training is finalized either by pushing the button on the ESP or using the Web interface a summary JSON file with all data is sent via REST API
* REST API (via Python Bottle) is used to interface between the Cassandra Database and the micro controller.

Of course, no one template will serve all projects since your needs may be different. So I'll be adding more in the near future. You may also suggest changes by forking this repo and creating a pull request or opening an issue.

A list of commonly used resources that I find helpful are listed in the acknowledgements.

### Built With
This section should list any major frameworks that you built your project using. Leave any add-ons/plugins for the acknowledgements section. Here are a few examples.
* [Arduino IDE](https://www.arduino.cc/en/main/software)
* [Cassandra](http://cassandra.apache.org/)
* [Bottle](https://bottlepy.org/docs/dev/)



<!-- GETTING STARTED -->
## Getting Started

XXX

### Prerequisites

There is a requirement to have internet in order to use this system. You can use a router connection or using your phone as a hotspot.

### Installation

1. Clone the repo
```sh
git clone https://github.com/your_username_/Project-Name.git
```
2. Install Arudino IDE
3. Install Cassandra Database
4. Start the REST API server

```python cassandraAPI.py```


<!-- USAGE EXAMPLES -->
## Usage

Use this space to show useful examples of how a project can be used. Additional screenshots, code examples and demos work well in this space. You may also link to more resources.

_For more examples, please refer to the [Documentation](https://example.com)_



<!-- ROADMAP -->
## Roadmap

See the [open issues](https://github.com/KristianDukov/Smart_gym/issues) for a list of proposed features (and known issues).


<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<!-- CONTACT -->
## Contact

Kristian Dukov - kris.dukov@gmail.com

Project Link: [https://github.com/your_username/repo_name](https://github.com/KristianDukov/Smart_gym)
