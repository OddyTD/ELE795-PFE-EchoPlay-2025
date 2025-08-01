import asyncio, websockets, json
from joueurs import Joueur
from utils import paquet, formater_nom_carte
from messages import Message
from session import SessionBlackjack

PORT = 8765
session = SessionBlackjack()

async def handler(websocket):
    if not session.ajouter_joueur(websocket):
        print(f"[SERVEUR] 🚫 Connexion refusée : trop de joueurs connectés")
        await websocket.send(Message.connexion_refusee())
        await websocket.close(reason="Nombre maximal de joueurs atteint")
        return

    joueur = session.get_joueur(websocket)
    print(f"[SERVEUR] ✅ {joueur.nom} connecté : {websocket.remote_address}")
    await Message.connexion_acceptee(websocket, joueur.nom)

    try:
        async for message in websocket:
            try:
                data = json.loads(message)
            except json.JSONDecodeError:
                print("[SERVEUR] ⚠️ Message non JSON")
                continue

            action = data.get("action")
            joueur = session.get_joueur(websocket)
            if not joueur:
                continue

            if action == "pret":
                if paquet.taille() < 2:
                    await Message.deck_epuise(websocket)
                    return

                cartes = paquet.tirer(2)
                joueur.ajouter_cartes(cartes)
                joueur.est_pret = True

                if joueur.est_blackjack():
                    print(f"[SERVEUR] 🎉 {joueur.nom} a un Blackjack !")
                    await Message.fin_partie(websocket, "Blackjack")
                    return

                noms = [formater_nom_carte(i) for i in cartes]
                print(f"[SERVEUR] ✋ {joueur.nom} main initiale : {noms}")
                await Message.main_initiale(websocket, cartes)
                print("[SERVEUR/DEBUG] 📋 |", joueur)

            elif action == "tirer_carte":
                if joueur.a_atteint_limite():
                    print("[SERVEUR] 🚫 Trop de cartes, requête ignorée")
                    continue

                if paquet.est_vide():
                    await Message.deck_epuise(websocket)
                    continue

                nouvelle_carte = paquet.tirer(1)[0]
                joueur.ajouter_cartes([nouvelle_carte])

                nom = formater_nom_carte(nouvelle_carte)
                print(f"[SERVEUR] 🃏 {joueur.nom} a tiré : {nom}")
                await Message.carte_envoyee(websocket, nouvelle_carte)
                print("[SERVEUR/DEBUG] 📋 |", joueur)

                if joueur.valeur_totale() > 21:
                    print(f"[SERVEUR] 💥 {joueur.nom} a dépassé 21 (score = {joueur.valeur_totale()}) → Défaite")
                    await Message.fin_partie(websocket, "Defaite")

                elif joueur.valeur_totale() == 21:
                    print(f"[SERVEUR] 🏆 {joueur.nom} atteint 21 points → Victoire")
                    await Message.fin_partie(websocket, "Blackjack")

            elif action == "rejouer":
              if joueur:
                  session.joueur_veut_rejouer(websocket)

                  print(f"🔁 {joueur.nom} veut rejouer")

                  if session.tous_veulent_rejouer():
                      print(f"[SERVEUR] 🔁 Tous les joueurs veulent rejouer. Nouvelle partie.")
                      session.reset()

                      # Notifie les deux joueurs
                      for j in session.joueurs.values():
                          await Message.main_vide(j.websocket)
                          await Message.reinitialisation(j.websocket)


    except websockets.exceptions.ConnectionClosed:
        print(f"[SERVEUR] ❌ Joueur déconnecté : {websocket.remote_address}")
    finally:
        session.retirer_joueur(websocket)

async def main():
    print(f"[SERVEUR] 🚀 Démarrage du serveur WebSocket sur le port {PORT}...")
    async with websockets.serve(handler, "0.0.0.0", PORT):
        await asyncio.Future()

if __name__ == "__main__":
    asyncio.run(main())
