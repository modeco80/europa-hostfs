GAME := starfighter
REGION ?= usa

GCC_VERSION ?= 2.96-ee-001003-1

# US version
GAME_starfighter_usa_VALID = yes

# sanity check
ifneq ($(GAME_$(GAME)_$(REGION)_VALID),yes)
$(error The game/version combination $(GAME)/$(REGION) is currently not supported.)
endif
