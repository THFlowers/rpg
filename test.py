import rpg
from rpg.constants import *
from random import randrange

sprite = rpg.Sprite("media/sprites/pdn4.bmp")
sprite.SetTile(randrange(0, (640/32)), randrange(0, (480/32)))
sprite.animation=randrange(0, 4)
