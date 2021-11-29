# ROIVert

![Screenshot](/screenshot.png)

---

## Contents
  - [What: What is ROIVert? (functional)](#what-what-is-roivert-functional)
  - [What: What is ROIVert? (internal)](#what-what-is-roivert-source)
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

&nbsp;&nbsp;

## What: What is ROIVert? (source)
ROIVert is a (soon multiplatform) C++ based application that emphasizes ease of use and performance. A [Qt](https://www.qt.io/) front-end allows manipulation of a variety of controls and display of plots, and an [OpenCV](https://opencv.org/) back-end provides quick processing.

&nbsp;&nbsp;

## Why: ROIVert's history
Some friends of ROIVert's original author at [Cornell](https://nbb.cornell.edu/) (and [Hobart and William Smith Colleges](https://www2.hws.edu/academics/biology/)) were working on a epifluorescence microscope. The idea was that they could have a microscope that they could use for [calcium imaging](https://en.wikipedia.org/wiki/Calcium_imaging) in lab classes.

Traditionally, these kinds of experiments use some pretty advanced (i.e. expensive) equipment, which means that only a handful of undergraduates get exposure to the methods (those that work in research labs). But the actual hardware requirements are pretty minimal, so it seemed feasible to build something inexpensive that could provide access to students (including those from disadvantaged backgrounds).

One of these profs wrote:
>The next phase of the project is to quantitate the imaging data. I was wondering if you knew of a user-friendly program (that is also inexpensive or free) to accomplish this? 

Sadly the answer was no. There's certainly free software out there, but it tends to be difficult to use. Indeed, this is generally true for scientific software (not just the free stuff). Folks in academia are happy to whip up some code to tackle computational problems. But software engineering doesn't seem to enter the mix, and the user interface in scientific software is typically unengineered, unemphasized, and unfriendly.

ROIVert's creator wrote:
>My gut reaction was that the free program for analyzing imaging data is [imagej](https://imagej.nih.gov/ij/), which is pretty unfriendly. I guess free and unfriendly normally go hand-in-hand in software? My other reaction was - maybe I could write something for you guys! 

And with that, ROIVert was born!

&nbsp;&nbsp;

## How: How to pronounce ROIVert
Pronounce ROIVert however you like, we're not pronounciation snobs here. The authors use all three of these pronounciations (click the IPA phonetics below to be redirected to a nifty phonetic reader):
 - French: (Green King) [ʁwa vɛʁ](http://ipa-reader.xyz/?text=%CA%81wa%20v%C9%9B%CA%81&voice=Mathieu)
 - English: [rɔɪ vɜrt](http://ipa-reader.xyz/?text=r%C9%94%C9%AA%20v%C9%9Crt&voice=Joey)
 - English: [ɑr oʊ aɪ vɜrt](http://ipa-reader.xyz/?text=%C9%91r%20o%CA%8A%20a%C9%AA%20v%C9%9Crt&voice=Russell)

&nbsp;&nbsp;

## How: How to run ROIVert
If you're not interested in the code, and instead want to use ROIVert, we suggest you head over to [roivert.net](http://roivert.net). We have the same binaries here, but the ROIVert site is easier to use and you'll find tutorials there too.

&nbsp;&nbsp;

## How: How to build
ROIVert relies on a standard CMake build system, but work is in progress to strengthen the CMake files and make ROIVert more easily ported. ROIVert depends on Qt5 (v1.0beta used 5.15.0) and OpenCV (v1.0beta used OpenCV440). Successful prototype builds have been achieved on macOS and linux.

The current plan is to establish a successful build process for MacOS and then work on abstracting some tooling for makefile generation. Early attempts with a CMake based approach were successful in Windows and linux.

If you'd like to build and are having trouble, reach out to [repo name]@[repo name].net

&nbsp;&nbsp;

## When: Release roadmap
A list of other major changes in ROIVert over the prototype version can be found at http://roivert.net/releasenotes.html

Next major feature targets are:
 - internal: Test framework.
 - external: Image stabalization, chart aspect ratio.

&nbsp;&nbsp;

## How: How to get Involved
ROIVert needs you! ROIVert was written using the one-guy-in-his-spare-time model. We're open to help in all ways, but currently seeking some specific expertise. If you can help please contact us at [repo name]@[repo name].net

- **Image Processing**: Image stablization and automatic blob detection are the next big target features for ROIVert. Help is needed at all levels (algorithm, ui design, etc.).
- **Documentation**: ROIVert needs content creators who can help teach how to be an effective user.
 - **Test**: ROIVert's test are under development. 
 - **Site**: The [roivert.net](http://www.roivert.net) site needs some actual design! (and some html skills).
 - **Translation**: We'd like to internationalize ROIVert and provide translation of ROIVert's messages. To do this, we need some multi-lingual folks!
 - **Code Signing**: ROIVert now can be distributed on macOS (release coming soon), but work is needed to sign ROIVert for Windows.

&nbsp;&nbsp;
 
## License
ROIVert is provided under the MIT license.

ROIVert includes dependencies on some other open source libraries (OpenCV and Qt). See [license.txt](license.txt) for more info. 

The author(s) of ROIVert are not lawyers, and while we can wrap our heads around neuroscience, microscopy, software design and implementation, the requirements of licensing totally elude us. If we've inadvertently violated licensing requirements please contact us at [repo name]@[repo name].net and we'll immediately act to correct it. 

&nbsp;&nbsp;

<center>
<a href = "http://roivert.net"><img src="/icons/GreenCrown.png" /></a>
</center>
