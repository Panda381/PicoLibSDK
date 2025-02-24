Video converter to PicoPad/DemoVGA format.

How to convert video to PicoPad format of video:

1) You will need programs: XMedia Recode, VirtualDub and Photoshop or another
   graphics editor that allows batch conversion of images to a BMP paletted file
   (like Gimp).

2) Convert some video to AVI format using XMeda Recode:
   - open video
   - on tab "Format" select output format "AVI", stream type "Video and Audio"
   - on tab "Video" select Convert to XviD with bitrate about 5000
   - on tab "Audio" select Convert to PCM 16-bit
   - on tab "Filters/Preview you can select part of video using mark
     of start frame and end frame. It is better to select a longer section
     and make a more precise crop in VirtualDub.
   - on bottom on window select Output to Input Folder / Don't override
   - on top of window in tool bar press "Add to queue" and "Encode"

3) Open video in VirtualDub.

   To select part of video:
   - if needed, select part of video with more precision. Shift pointer to start
     of selection, press "Mark Out" button (last button on bottom toolbar - left
     part gets lighter), press Delete to cut left part. Shift pointer to end
     of selection, press "Mark In" button. Press button "End" and then press
     button "Mark Out" - right part gets lighter. Press Delete to delete right
     part. Press button "Start" to move pointer to start of video (is required,
     otherwise an error may occur if the pointer remains after the last frame).

   To convert to 10 fps and to 160x240:
   - open menu Video / Filters. Add "interpolate" filter. Set target frame
     rate to 10 fps, using "Linear blending" interpolation mode. OK close filter.
     Add filter "resize" (must be after "interpolate" filter!). Set Aspect ratio
     to "Disabled". Set new size to 160 x 240 pixels. Press OK. If the video has
     other aspect ratio than 4:3, press Cropping and crop video to ratio about
     4:3. Filter "Close Filters by OK.
   - open menu File / Save as AVI and export video to some temporary video.

   To export video to images:
   - open temporary video
   - open menu Video / Filters and delete 'interpolate' and "resize" filters.
   - create directory "BMP"
   - open menu File / Export / Image sequence. Select output format
     "Windows BMP". Select "directory to hold" to the "BMP" directory.
     Set Minimum number of digits to 6. Set OK to export images.

   To export sound:
   - select menu Audio / Full processing mode
   - open menu Audio / Conversion. Select Sampling rate = 22050 Hz,
     High quality, Precision = 8-bit, Channels = Mono. Close with OK.
   - save sound to WAV using menu File / Save WAV, set filename SOUND.wav.

4) Open Photoshop, to convert images to 8-bit palettes:
   - copy first file 000000.bmp in the BMP directory elsewhere, as sample.
   - open the copied file 000000.bmp in Photohop
   - open panel Actions. Create new action (Action 1). It will start recording.
   - convert image to palettes: select menu Image / Mode / Indexed Color.
     Select Palette to "Local (Adaptive)", set Colors to 256, set Diffusion
     to amount 75%. Press OK to convert.
   - save image using menu Image / Save As. Press Save, overwrite yes.
     In next window select Windows format, Depth = 8 bit. No compression.
     Check "ON" option "Flop row order". Press OK.
   - close file with File / Close.
   - stop recording Action
   - now you can delete copied file 000000.bmp
   - Run batch conversion of images using menu File / Automate / Batch.
     In next window select your new create action Action 1. Set source
     to "Folder". Select folder "BMP" with images using button Choose.
     Option "Override Action Save As" should be enabled. Press OK to start.

5) Convert video to PicoPad format:
   - Run the PicoPadVideo program. It requires file "SOUND.wav" in its
     current folder and images in "BMP" subfolder, with resolution 160x240
     pixels, paletted BMP with 256 palettes, flip row order, names with
     6 digits, starting with name 000000.bmp. It will output result file
     to the name VIDEO.VID. You can rename output file as you wish.
