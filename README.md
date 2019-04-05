#### Visocyte build and install on Ubuntu 18.04
1. sudo apt install qtxmlpatterns5-dev-tools
2. export QT_SELECT=qt5 (if you don’t this, make -jx will fail)
3. git clone --recursive git@github.com:/satya-arjunan/visocyte.git
4. cd visocyte
5. mkdir build
6. cd build
7. cmake ..
8. make -j16

