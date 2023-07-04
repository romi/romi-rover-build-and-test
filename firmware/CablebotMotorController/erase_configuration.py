import odrive

odrv = odrive.find_any(serial_number="316133753232")
axis = odrv.axis0

try:
    odrv.erase_configuration()
except:
    print("Returns error: probably okay(?)")

    
