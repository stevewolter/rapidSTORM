# Summary

The rapidSTORM project is an open source evaluation tool that provides fast and highly configurable data processing for single-molecule localization microscopy such as dSTORM. It provides both two-dimensional and three-dimensional, multi-color data analysis as well as a wide range of filtering and image generation capabilities.

rapidSTORM is currently available in two versions: rapidSTORM 2 is the current stable branch, where only outstanding bugs are fixed, while rapidSTORM 3 is the unstable development branch with an improved user interface and simplified technical deployment.

# News, updates and discussion

You can track announcements, news and new releases for rapidSTORM by subscribing to the rapidstorm-announce mailing list. This list is very low volume (typically less than one mail per month) and read-only. Your questions, feedback, and discussions about rapidSTORM are very welcome on the rapidstorm-discuss list.

# Manual

The online manual is at [https://storage.googleapis.com/rapidstorm/doc/index.html](https://storage.googleapis.com/rapidstorm/doc/index.html).

# Download and installation

  * **Windows**: Download and run the [RapidSTORM 3.3.1 64 bit installer](https://storage.googleapis.com/rapidstorm/binary-win64/rapidstorm-3.3.1-win64.exe) for Windows. For older systems, there's a [32 bit installer](https://storage.googleapis.com/rapidstorm/binary-win32/rapidstorm-3.3.1-win32.exe). Older versions are linked from the [index page](http://storage.googleapis.com/rapidstorm).
  * **Debian Linux**:
    * Currently, we support Debian distributions **stretch** (stable), **wheezy** and **squeeze**.
    * Add my public key to your keyring: 

        ```
        gpg --keyserver pgp.mit.edu --export 70CCA3637EA8E63E71F1DA0F96B1CF3AF02E1BAB
        ```

    * Add the Rapidstorm debian repository for your distribution:

        ```
        DIST=wheezy
        sudo tee /etc/apt/sources.list.d/rapidstorm.list <<EOF
        deb https://storage.googleapis.com/rapidstorm/debian ${DIST} main contrib non-free
        deb-src https://storage.googleapis.com/rapidstorm/debian ${DIST} main contrib non-free
        EOF
        sudo apt-get update
        ```

    * Install rapidstorm: `sudo apt-get install rapidstorm`
  * **Ubuntu**:
    * We support the Ubuntu distributions **precise**, **saucy**, **trusty** and **xenial**. Follow the instructions for Debian above and
      replace "debian" with "ubuntu" everywhere.
  * **Source**: Download the source from the [GitHub repository](https://github.com/stevewolter/rapidSTORM) and follow the instructions in the [INSTALL file](https://github.com/stevewolter/rapidSTORM/blob/master/INSTALL).

# rapidSTORM features

  * Basic features
    * Graphical user interface
    * Command line interface
    * Easy scriptability and testability through simparm framework
    * Reading Andor SIF files
    * Reading and writing TIFF files
    * Direct connection to Andor cameras
    * Pseudorandom input data simulation with true Besselian PSFs
    * Input simulation for 3D PSFs
    * Least-squares fitting
    * Data analysis in seconds
    * Modular, easily extensible architecture
  * rapidSTORM processing features
    * Least-squares fitting
    * MLE fitting
    * 2D processing with known PSF
    * 2D fitting with per-spot PSF width
    * Estimation of PSF from input data
    * Estimation of 3D PSF from input data
    * 3D using astigmatism
    * 3D using biplane
    * Multi-color and multi-plane support
    * Two-kernel analysis for multi-spot events
  * rapidSTORM result analysis features
    * Online result view during analysis
    * High & adaptable image contrast through WHN
    * Time-resolved images
    * Can create time-lapse movies
    * Free selection of coordinate and hue axes in image generation
    * Online filtering of results during analysis
    * Advanced result filtering using arithmetic expressions
    * Easy linear drift correction
    * Joining multiple time-consecutive localizations by Kalman tracking
