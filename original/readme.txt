Hello,

This is the third public release of my Hi-Colour gameboy convertor.

8th April 2000

v1.2 I added a few other quantisers into the convertor, experiment with them
one type of quantiser does not stand head and shoulders above the rest, some
of them give better results than others for certain types of picture. The
Median cut with dither, I think, produces the best results most of the
time when used with adaptive 3 for left and right sides of the screen.

30 March 2000

v1.1 Rob jones fixed windows startup problems and added thread support for 
the convertor.

27th March 2000

v1.0 First public release


There is an example .GB file already created, I did not include the original
image so that the .zip file is as small as possible. If you run the compile
batch file, this demo GB image will be lost. This picture was converted using
adaptive 3 - Wu Conversion.

This convertor will convert a 24 bit 160x144 TGA image into a set of data
files that the GBC can display. Everything is included in this package in
order for you to create a .GB file. The only additional files that you will
require are the RGBDS assembler tools.

Just follow these simple steps to get a HiColour image onto your GBC.


1) Run the Hicolour.exe windows program

2) Select a TGA 24 bit 160x144 image to convert. The image will be displayed
   in the top left box.

3) Select conversion type for each side of screen.Adaptive 3 is the best
   quality, but quite SLOW. Good results can be had from most of the other
   options within the pull down boxes.

4) Select a quantise method to use.

5) Hit the convert button.

6) Save the data, using default filename (test) in the directory that the code
   is in.

7) The True Gameboy Colour button will allow you to see how the image should
   look on the GBC.

8) Run Compile.bat

9) Send the .GB image to the gameboy / emulator.


More Advanced use (tweaking the image).

This is the best method for altering RGB values and seeing the results faster.

Select Adaptive 3 for both sides of the screen, keep the RGB sliders in the
middle of their ranges.

Hit the convert button.

Make a cup of tea ;)

Once the image is converted, select the view Attribute button.

Make a note of the left hand side, attribute selection. This will be a
constant for the whole of the left side of the screen.

Close the message box.

Pull down the left hand attribute selection box, and select this conversion
format, keep the right hand side as adaptive 3. This will speed up the
conversion process when you re-convert an image.

Tweak the RGB sliders, pressing the convert button, will re-convert the image
using the new RGB settings.

Once you are happy with the image, save it.

Run the batch file.

Have Fun.

Let me know what you think.

Any comments / critisism welcome.

GlenCook@hotmail.com

p.s. Full source for the project (visual C v6) should be available at any
GB site that hosts this type of file.

Feel free to use this code any way you feel fit.

You can use this code / algorithm in any commercial product. It would be great
if you could send me an e-mail letting me know.


And now for the usual bull..

This software is supplied on an as-is basis, although I don't believe that
there are any bugs in my code, I will not be held responsible for any damage
this program may cause....


Glen Cook.


Conversion times, Athlon 750, pc-133

                fixed line    Adaptive-1   Adaptive-2   Adaptive-3

Original           1.2           5.5         13.2          396.8  

Median             1.45          4.8         7.4            54.2

Median Dith        1.45          4.8         7.7            55.4

Wu                 7.3          25.2        39.8           292.6
 
