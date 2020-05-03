import rpg
from rpg.constants import *

print rpg.test()

sprite = rpg.Sprite("media/sprites/wmg3.bmp")
sprite.SetTile(2, 7)
sprite.animation=DOWN
sprite.SetAI("RANDOM");

sprite2 = rpg.Sprite("media/sprites/pdn4.bmp")
sprite2.SetTile(9, 0)
sprite2.animation=DOWN
sprite2.SetAI("RANDOM")

sprite3 = rpg.Sprite("media/sprites/wmg2.bmp")
sprite3.SetTile(10, 10)
sprite3.SetAI("NULL")
