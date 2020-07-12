
rcom request cnc '{"command": "???"}'
rcom request cnc '{"command": "moveto"}'
rcom request cnc '{"command": "moveto", "x": 1000}'
rcom request cnc '{"command": "moveto", "x": -1000}'
rcom request cnc '{"command": "moveto", "y": 1000}'
rcom request cnc '{"command": "moveto", "y": -1000}'
rcom request cnc '{"command": "moveto", "z": 1000}'
rcom request cnc '{"command": "moveto", "z": -1000}'
rcom request cnc '{"command": "spindle"}'
rcom request cnc '{"command": "spindle", "speed": -1}'
rcom request cnc '{"command": "spindle", "speed": 1000}'
rcom request cnc '{"command": "travel"}'
rcom request cnc '{"command": "travel", "path": []}'
rcom request cnc '{"command": "travel", "path": [[]]}'
rcom request cnc '{"command": "travel", "path": [[0,0]]}'
rcom request cnc '{"command": "travel", "path": [[1000,-1000,1000]]}'


rcom request cnc '{"command": "moveto", "x": 0.1, "y": 0.1}'
rcom request cnc '{"command": "spindle", "speed": 1}'
sleep 3
rcom request cnc '{"command": "spindle", "speed": 0}'
rcom request cnc '{"command": "moveto", "x": 0, "y": 0}'
rcom request cnc '{"command": "homing"}'
rcom request cnc '{"command": "travel", "path": [[0,0,0], [0.5,0,0], [0.5,0.5,0], [0,0.5,0], [0,0,0]]}'

