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
ROIVert is a multiplatform C++ based application that emphasizes ease of use and performance. A [Qt](https://www.qt.io/) front-end allows manipulation of a variety of controls and display of plots, and an [OpenCV](https://opencv.org/) back-end provides quick processing. ROIVert is backed by a full test suite and an engineered UI.

&nbsp;&nbsp;

## Why: ROIVert's history
Some friends of ROIVert's author at [Cornell](https://nbb.cornell.edu/) (and [Hobart and William Smith Colleges](https://www2.hws.edu/academics/biology/)) were working on a epifluorescence microscope. The idea was that they could have a microscope that they could use for [calcium imaging](https://en.wikipedia.org/wiki/Calcium_imaging) in lab classes.

Traditionally, these kinds of experiments use some pretty advanced (i.e. expensive) equipment, which means that only a handful of undergraduates get exposure to the methods (those that work in research labs). But the actual hardware requirements are pretty minimal, so it seemed feasible to build something inexpensive that could provide access to students (including those from disadvantaged backgrounds).

One of these profs wrote:
>The next phase of the project is to quantitate the imaging data. I was wondering if you knew of a user-friendly program (that is also inexpensive or free) to accomplish this? 

Sadly the answer was no. There's certainly free software out there, but it tends to be difficult to use. General image processing tools for biologists provide massive power, but their interface is intimidating. Other custom tools written in high-level numeric analysis scripting languages can be slow, difficult to install, and prone to bugs.

ROIVert's creator wrote:
>My gut reaction was that the free program for analyzing imaging data is [imagej](https://imagej.nih.gov/ij/), which is pretty unfriendly. There are probably some tools in Python and MATLAB, but I'd guess they'd be slow and painful to install, and maybe a little buggy. My other reaction was - maybe I could write something for you guys! 

And with that, ROIVert was born!

&nbsp;&nbsp;

## How: How to pronounce ROIVert
Pronounce ROIVert however you like, we're not pronounciation snobs here. The authors use all three of these pronounciations (click the IPA phonetics below to be redirected to a nifty phonetic reader):
 - French: (Green King) [ʁwa vɛʁ](http://ipa-reader.xyz/?text=%CA%81wa%20v%C9%9B%CA%81&voice=Mathieu)
 - English: [rɔɪ vɜrt](http://ipa-reader.xyz/?text=r%C9%94%C9%AA%20v%C9%9Crt&voice=Joey)
 - English: [ɑr oʊ aɪ vɜrt](http://ipa-reader.xyz/?text=%C9%91r%20o%CA%8A%20a%C9%AA%20v%C9%9Crt&voice=Russell)

&nbsp;&nbsp;

## How: How to run ROIVert
If you're not interested in the code, and instead want to use ROIVert, head over to [roivert.net](http://roivert.net). We have the same binaries here, but the ROIVert site is easier to use and you'll find tutorials there too.

&nbsp;&nbsp;

## How: How to build
ROIVert relies on a CMake build system. ROIVert depends on Qt5 (tested with 5.15.0 and 5.15.2), OpenCV (tested with OpenCV 4.4.0), and TinyTIFF. Version 1.1 was built on Windows, Linux, and MacOS. If you'd like to build and are having trouble, reach out to [repo name]@[repo name].net. ROIVert comes with a full test suite which can be run directly via CLI (use ROIVertTest.exe -h for more information).

&nbsp;&nbsp;

## When: Release roadmap
A list of other major changes in ROIVert over the prototype version can be found at http://roivert.net/releasenotes.html

Next major feature targets are:
 - Image stabalization/motion correction
 - Dark mode for Windows
 - Color palettes
 - Image chart
 
&nbsp;&nbsp;

## How: How to get Involved
ROIVert needs you! If you can help with development, documentation, testing, translation, or web design please contact us at [repo name]@[repo name].net

&nbsp;&nbsp;
 
## License
ROIVert is provided under the MIT license.

ROIVert includes dependencies on some other open source libraries (OpenCV, Qt, TinyTiff). See [license.txt](license.txt) for more info. 

If we've inadvertently violated licensing requirements please contact us at [repo name]@[repo name].net and we'll immediately act to correct our misuse. 

&nbsp;&nbsp;

<center>
<a href = "http://roivert.net"><img src="/icons/GreenCrown.png" /></a>
</center>
