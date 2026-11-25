NAME = "lab8"

LOCAL_OPTIMA = 1000

# 1000 local optima: greedy local search (inter-route exchange + two-edges exchange)
# started from random solutions, one seed per run.

def generate():
    yield ("Greedy Local Search from random solutions", [[
        [],
        ("RandomSolver", {"seed": seed}),
        ("LocalSearch", {"searchType": "Greedy", "intraMove": "Edges"})
    ] for seed in range(LOCAL_OPTIMA)])

def measurements():
    yield ("Local search steps", lambda d: d.get("Steps"), lambda v: f"{v:,}".replace(",","\\,"))
