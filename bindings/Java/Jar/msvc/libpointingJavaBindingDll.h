// Le bloc ifdef suivant est la façon standard de créer des macros qui facilitent l'exportation 
// à partir d'une DLL. Tous les fichiers contenus dans cette DLL sont compilés avec le symbole LIBPOINTINGJAVABINDINGDLL_EXPORTS
// défini sur la ligne de commande. Ce symbole ne doit pas être défini dans les projets
// qui utilisent cette DLL. De cette manière, les autres projets dont les fichiers sources comprennent ce fichier considèrent les fonctions 
// LIBPOINTINGJAVABINDINGDLL_API comme étant importées à partir d'une DLL, tandis que cette DLL considère les symboles
// définis avec cette macro comme étant exportés.
#ifdef LIBPOINTINGJAVABINDINGDLL_EXPORTS
#define LIBPOINTINGJAVABINDINGDLL_API __declspec(dllexport)
#else
#define LIBPOINTINGJAVABINDINGDLL_API __declspec(dllimport)
#endif

// Cette classe est exportée de libpointingJavaBindingDll.dll
class LIBPOINTINGJAVABINDINGDLL_API ClibpointingJavaBindingDll {
public:
	ClibpointingJavaBindingDll(void);
	// TODO : ajoutez ici vos méthodes.
};

extern LIBPOINTINGJAVABINDINGDLL_API int nlibpointingJavaBindingDll;

LIBPOINTINGJAVABINDINGDLL_API int fnlibpointingJavaBindingDll(void);
