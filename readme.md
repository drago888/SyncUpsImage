#SyncUpsImage#
##What does Sync Upscaled Image do?
It compares the images upscaled by ESRGanResizer with the source image and change the destination image extension to match the source image.
If the source image has transparent pixels(will treat 8 bits images with value 0 as transparent), will also modify the destination image to match it.

##Where do I get the deps library and executable file?##
You can get it from the link below
[Library & binary](https://drive.google.com/drive/folders/1tMoPEqmbcM9h0Rtr7Pu2D3pqcng8tGqo?usp=sharing)

##What is the scale parameter?##
It is the scale factor use to upscale the factor in ESRGanResizer. Eg 4 for 4x

##How to use it?##
Run the below command
  SyncUpsImage "C:\dest" "c:\src" 4
