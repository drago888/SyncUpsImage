# SyncUpsImage
## What does Sync Upscaled Image do?
It compares the images upscaled by ESRGanResizer with the source image and change the destination image extension to match the source image.  
If the source image has transparent pixels(will treat 8 bits images with value 0 as transparent), will also modify the destination image to match it.

It is used in conjunction with Trans2Png. Run Trans2Png to convert transparent background to background color before passing to ESRGanResizer.  
This is due to ESRGan unable to process transparent pixel properly.

## Where do I get the deps library and executable file?
You can get it from the link below  
[Library & binary - https://drive.google.com/drive/folders/1tMoPEqmbcM9h0Rtr7Pu2D3pqcng8tGqo?usp=sharing](https://drive.google.com/drive/folders/1tMoPEqmbcM9h0Rtr7Pu2D3pqcng8tGqo?usp=sharing)

## What is the scale parameter (Third parameter)?
It is the scale factor use to upscale the factor in ESRGanResizer. Eg 4 for 4x

## What is the tolerance parameter(Fourth parameter)?
It is the tolerance for deviation from the background color. With 0 tolerance, it must be exactly the background color before it is converted to transparent  
For the images that I am using, I find that tolerance of 150 gives quite a good result.

## How to use it?
Run the below command  
  SyncUpsImage "C:\dest" "c:\src" 4 2

First parameter is the destination directory of the images to be modified.
Second parameter is the source directory of original images to be compared with.
Third parameter is the scale factor. Thus for 4x scaling, it will be 4.
Fourth parameter is the tolerance for transparency. 
