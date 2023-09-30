# Place this file in the kritarunner folder in AppData or similar

from krita import Krita, InfoObject
from collections import deque
import subprocess
import os
import time

# Generates the ponygame "tex" file, storing the metadata needed to run
# this whole process.
#
# This includes all the animation info.
def generate_tex_file(tex_path, kra_path, png_path, frame_width, frame_height, frame_length, frame_count):
    with open(tex_path, "w") as output:
        output.write(f'@load {png_path}\n')
        output.write(f'@from-krita {kra_path}\n')
        output.write('@anim loop\n')
        
        x = 0
        y = 0
        
        for i in range(0, frame_count):
            output.write(f'\t@frame {frame_length}ms {x} {y} {frame_width} {frame_height}\n')
            x += frame_width

# Arguments we need:
# -> .kra file to process
# -> folder to save temp .pngs in
# -> output name for spritesheet .png
# -> output name for .tex file
def __main__(args):    
    # test args
    # kra_file_path = "test_krita.kra"
    # tmp_folder_path = ".ponytmp"
    # spritesheet_png_path = "output_sheet.tex.png"
    # tex_path = "output_sheet.tex"
    
    app = Krita.instance()
    app.setBatchmode(True) # Extremely necessary or everything blows up
    #print("GOING TO SLEEP")
    #
    #print(win)
    
    if len(args) != 4:
        print("wrong arg count passed -- perhaps invoked incorrectly?")
        print("(should be using kritarunner)")
        return
    
    kra_file_path = args[0]
    tmp_folder_path = args[1]
    spritesheet_png_path = args[2]
    tex_path = args[3]
    
    try:
        os.mkdir(tmp_folder_path)
    except OSError as error:
        print(error)
    
    doc = app.openDocument(kra_file_path)
    win = app.openWindow() # For toggling off the onion skin
    
    
    # Toggle off onion skins
    app.action('toggle_onion_skin').trigger()
    
   
    first_frame = doc.fullClipRangeStartTime()
    last_frame = doc.fullClipRangeEndTime()
    png_options = InfoObject()
    png_options.setProperties({
        "alpha": True,
        "compression": 1, # Small compression to save time
        "forceSRGB": False,
        "indexed": False,
        "interlaced": False,
        "saveSRGBProfile": False,
        "transparencyFillcolor": [255,255,255]
            })

    frame_and_path_list = list()
    magick_args = ['magick', 'convert']
    for frame in range(first_frame, last_frame + 1):
        # Generate export file names
        next_export_file = f'{tmp_folder_path}\\E{frame}.png'
        
        # Store all export files in list for later
        frame_and_path_list.append((frame, next_export_file))
        
        # Add export files to magick args for concatenation as well
        magick_args.append(next_export_file)

    # Export all the frames. This would be a function but apparently that
    # breaks things.
    doc.setBatchmode(True)
    q = deque(frame_and_path_list)
    
    failure_count = 0
    max_failures = len(frame_and_path_list) * 2
    
    # Use a deque to repeatedly try each image.
    # This seems to not be necessary anymore, now that we're calling
    # openWindow, but before, the images would sometimes fail to write.
    while len(q) != 0:
        next_pair = q.popleft()
        frame = next_pair[0]
        path = next_pair[1]
        print(f"generating image {path} for {frame}")
        
        doc.setCurrentTime(frame)
        should_unlock = False
        for i in range(0, 100):
            if doc.tryBarrierLock():
                should_unlock = True
                break
        success = doc.exportImage(path, png_options)
        if should_unlock:
            doc.unlock()
        
        if not success:
            q.append(next_pair)
            failure_count += 1
            
            if failure_count > max_failures:
                print('too many failures')
                return

    print(f'number of failures = {failure_count}')
       
    # Now, generate the tex file. This is done before imagemagick because it's fast.
    frame_count = len(frame_and_path_list)
    
    fps = doc.framesPerSecond()
    # Ms per frame unfortunately is forced to be an int right now.
    ms_per_frame = int(1000 / fps)
    
    print("generating tex file")
    generate_tex_file(tex_path, kra_file_path, spritesheet_png_path, doc.width(), doc.height(), ms_per_frame, frame_count)
    
    # Setup imagemagick final arguments: appending to a horizontal spritesheet,
    # with the given output file.
    magick_args.append('+append')
    magick_args.append(spritesheet_png_path)

    print("generating spritesheet")
    # Build the spritesheet by concatenating the generated files.
    subprocess.run(magick_args)