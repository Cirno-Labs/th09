import 'tools/common.just'

# Link the exe's objects in natural order, without the layout hook, and
# count the game functions at their original address (N: how many
# misplaced ones to list)
natural N="10": exe
    python3 scripts/natural_link.py {{N}}
