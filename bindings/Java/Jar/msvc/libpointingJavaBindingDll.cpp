// libpointingJavaBindingDll.cpp : définit les fonctions exportées pour l'application DLL.
//

#include "stdafx.h"
#include "libpointingJavaBindingDll.h"


// Il s'agit d'un exemple de variable exportée
LIBPOINTINGJAVABINDINGDLL_API int nlibpointingJavaBindingDll=0;

// Il s'agit d'un exemple de fonction exportée.
LIBPOINTINGJAVABINDINGDLL_API int fnlibpointingJavaBindingDll(void)
{
	return 42;
}

// Il s'agit du constructeur d'une classe qui a été exportée.
// consultez libpointingJavaBindingDll.h pour la définition de la classe
ClibpointingJavaBindingDll::ClibpointingJavaBindingDll()
{
	return;
}
