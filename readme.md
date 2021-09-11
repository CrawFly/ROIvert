# ROIVert

![Screenshot](/screenshot.png)

---

## Contents
  - [What: What is ROIVert? (functional)](#what-what-is-roivert-functional)
  - [What: What is ROIVert? (internal)](#what-what-is-roivert-internal)
  - [Why: ROIVert's history](#why-roiverts-history)
  - [How: How to pronounce ROIVert](#how-how-to-pronounce-roivert)
  - [How: How to run ROIVert](#how-how-to-run-roivert)
  - [How: How to build](#how-how-to-build)
  - [When: Release roadmap](#when-release-roadmap)
  - [How: How to get Involved](#how-how-to-get-involved)
  - [License](#license)

---


## What: What is ROIVert? (functional)
ROIVert is a software tool for drawing regions of interest of videos, and computing the (normalized) intensity of pixels in those regions over time. ROIVert was designed for the analysis of time-lapse microscopy, specifically for recordings of GCaMP from the fruit fly.  More information of roivert can be found at [roivert.net](http://roivert.net)

## What: What is ROIVert? (source)
ROIVert is a (soon multiplatform) C++ based application that emphasizes ease of use and performance. A [Qt](https://www.qt.io/) front-end allows manipulation of a variety of controls and display of plots, and an [OpenCV](https://opencv.org/) back-end provides quick processing.

## Why: ROIVert's history
Some friends of ROIVert's original author at [Cornell](https://nbb.cornell.edu/) (and [Hobart and William Smith Colleges](https://www2.hws.edu/academics/biology/)) were working on a epifluorescence microscope. The idea was that they could have a microscope that they could use for [calcium imaging](https://en.wikipedia.org/wiki/Calcium_imaging) in lab classes.

Traditionally, these kinds of experiments use some pretty advanced (i.e. expensive) equipment, which means that only a handful of undergraduates get exposure to the methods (those that work in research labs). But the actual hardware requirements are pretty minimal, so it seemed feasible to build something inexpensive that could provide access to students (including those from disadvantaged backgrounds).

One of these profs wrote:
>The next phase of the project is to quantitate the imaging data. I was wondering if you knew of a user-friendly program (that is also inexpensive or free) to accomplish this? 

Sadly the answer was no. There's certainly free software out there, but it tends to be difficult to use. Indeed, this is generally true for scientific software (not just the free stuff). Folks in academia are happy to whip up some code to tackle computational problems. But software engineering doesn't seem to enter the mix, and the user interface in scientific software is typically unengineered, unemphasized, and unfriendly.

ROIVert's creator wrote:
>My gut reaction was that the free program for analyzing imaging data is [imagej](https://imagej.nih.gov/ij/), which is pretty unfriendly. I guess free and unfriendly normally go hand-in-hand in software? My other reaction was - maybe I could write something for you guys! 

And with that, ROIVert was born!

## How: How to pronounce ROIVert
Pronounce ROIVert however you like, we're not pronounciation snobs here. The authors use all three of these pronounciations (click the IPA phonetics below to be redirected to a nifty phonetic reader):
 - French: (Green King) [ʁwa vɛʁ](http://ipa-reader.xyz/?text=%CA%81wa%20v%C9%9B%CA%81&voice=Mathieu)
 - English: [rɔɪ vɜrt](http://ipa-reader.xyz/?text=r%C9%94%C9%AA%20v%C9%9Crt&voice=Joey)
 - English: [ɑr oʊ aɪ vɜrt](http://ipa-reader.xyz/?text=%C9%91r%20o%CA%8A%20a%C9%AA%20v%C9%9Crt&voice=Russell)

## How: How to run ROIVert
If you're not interested in the code, and instead want to use ROIVert, we suggest you head over to [roivert.net](http://roivert.net). We have the same binaries here, but the ROIVert site is easier to use and you'll find tutorials there too.

## How: How to build
ROIVert currently doesn't have automated tools for makefile generation, but ROIVert's dependencies are simple: ROIVert is built with Qt5 (v1.0beta used 5.15.0) and OpenCV (v1.0beta used OpenCV 440). ROIVert has been built in Windows with MSVC (and built but not successfully linked with MingW), and in linux with GCC. 

The current plan is to establish a successful build process for MacOS and then work on abstracting some tooling for makefile generation. Early attempts with a CMake based approach were successful in Windows and linux.

If you'd like to build and are having trouble, reach out to [repo name]@[repo name].net

## When: Release roadmap
The primary goals for the initial release (over the prototype) were to build in a charting system to release ROIVert from dependencies on charts licensed under GPL. A list of other major changes in ROIVert over the prototype version can be found here [roivert.net](http://roivert.net/releasenotes.html) 

## How: How to get Involved
ROIVert needs you! ROIVert was written using the one-guy-in-his-spare-time model. We're open to help in all ways, but currently seeking some specific expertise. If you can help please contact us at [repo name]@[repo name].net

- **MacOs Build/Deploy**: ROIVert needs a MacOS Build. A release on Mac is currently our top priority but the authors have no experience with this. A successful linux build was achieved in September 2021 using wsl2 (but no attempts were made to package and release). We think we have all our 'ducks in a row' (modulo a mac to build on and a developer account).
- **Build System**: As ROIVert moves to multiplatform, we need to work on an automated build system. We've done some protoyping around thsi but could use CMake/QMake/prebuild expertise. 
- **Documentation**: We made some quick little tutorials with the 1.0 beta release, but we'd like to do better. We had some trouble setting up a wiki on the ROIVert site. ROIVert needs content creators who can help teach how to be effective using ROIVert. 
 - **Test**: ROIVert was built without tests. The initial prototype was built long before ROIVert was expected to be so broad as a project, and the first release was a bit rushed. Incorporation of a test framework will be essential to ROIVert's ability to grow. 
 - **Architecture**: ROIVert has some weak spots in its architecture, refactoring will be essential to ROIVert's ability to grow. We need some C++ and Qt expertise for this, particularly some folks who are willing to invest time in discussion.
 - **Site**: The [roivert.net](http://www.roivert.net) site needs some actual design! (and some html skills).
 - **Translation**: We'd like to internationalize ROIVert and provide translation of ROIVert's messages. To do this, we need some multi-lingual folks!
 - **$$$$**: ROIVert doesn't need money for profit, but we could use a few things that cost money: apple developer account, a better web host (we use the bottom-of-the-barrel), code signing for Microsoft.

 
## License
ROIVert is provided under the MIT license.

ROIVert includes dependencies on some other open source libraries (OpenCV and Qt). See [license.txt](./LICENSE.TXT) for more info. 

The author(s) of ROIVert are not lawyers, and while we can wrap our heads around neuroscience, microscopy, software design and implementation, the requirements of licensing totally elude us. If we've inadvertently violated licensing requirements please contact us at [repo name]@[repo name].net and we'll immediately act to correct it. 


<center>
<a href = "http://roivert.net"><img src="/icons/GreenCrown.png" /></a>
</center>