from utils import paquet
from joueurs import Joueur

"""Classe représentant une session de jeu de Blackjack"""
class SessionBlackjack:

    def __init__(self):
        self.joueurs = {}  # Dictionnaire {websocket: Joueur}

    def ajouter_joueur(self, websocket):
        """Ajoute un joueur si la session n’est pas pleine"""
        if websocket in self.joueurs:
            return True  # Déjà connecté

        if len(self.joueurs) >= 2:
            return False  # Limite atteinte

        nom = f"Joueur {len(self.joueurs) + 1}"
        self.joueurs[websocket] = Joueur(websocket, nom)
        print(f"[SERVEUR] 👤 Nouveau joueur ajouté : {nom}")
        return True

    def retirer_joueur(self, websocket):
        """Supprime un joueur de la session"""
        self.joueurs.pop(websocket, None)

    def get_joueur(self, websocket):
        """Retourne l'objet Joueur associé au websocket"""
        return self.joueurs.get(websocket)

    def get_etat_joueurs(self):
        """Retourne la liste des états des joueurs (debug)"""
        return [repr(j) for j in self.joueurs.values()]

    def tous_les_joueurs_ont_termine(self):
        """Vérifie si tous les joueurs ont terminé leur tour"""
        return all(j.a_termine for j in self.joueurs.values()) and len(self.joueurs) == 2

    def reset(self):
        """Réinitialise la session et le paquet"""
        for joueur in self.joueurs.values():
            joueur.reinitialiser()
        paquet.reinitialiser()
    
    def joueur_veut_rejouer(self, websocket):
      joueur = self.joueurs.get(websocket)
      if joueur:
          joueur.veut_rejouer = True

    def tous_veulent_rejouer(self):
      return len(self.joueurs) == 2 and all(j.veut_rejouer for j in self.joueurs.values())
