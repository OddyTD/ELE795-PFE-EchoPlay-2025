import json

class Message:

    @staticmethod
    async def main_initiale(websocket, cartes):
        await websocket.send(json.dumps({
            "action": "main_initiale",
            "cartes": cartes
        }))

    async def main_vide(websocket):
        await websocket.send(json.dumps({
            "action": "main_initiale",
            "cartes": []
        }))

    @staticmethod
    async def carte_envoyee(websocket, carte):
        await websocket.send(json.dumps({
            "action": "carte_envoyee",
            "cartes": [carte]
        }))

    @staticmethod
    async def deck_epuise(websocket):
        await websocket.send(json.dumps({
            "action": "deck_epuise",
            "message": "Pas assez de cartes dans le paquet"
        }))

    @staticmethod
    async def fin_partie(websocket, resultat):
        await websocket.send(json.dumps({
            "action": "fin_partie",
            "resultat": resultat
        }))

    @staticmethod
    async def connexion_refusee():
        return json.dumps({
            "action": "connexion_refusee",
            "message": "Nombre maximal de joueurs atteint"
        })

    @staticmethod
    async def connexion_acceptee(websocket, nom):
        await websocket.send(json.dumps({
            "action": "connexion_acceptee",
            "nom": nom
        }))

    @staticmethod
    async def reinitialisation(websocket):
        await websocket.send(json.dumps({
            "action": "reinitialisation"
        }))
