#Todo

[ ] Maybe we should replace event uid from char array to int (or other numeric type).
Only if we need some speed improvement.

[ ] Find a solution to allow joining a fsm which is in a direct loop step (without watching 
transition) maybe with the running var ? 0: stopped, 1: running, 2: stopping. 