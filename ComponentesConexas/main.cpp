#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

// Punto de entrada de la aplicación.
// Aunque el proyecto es una "CLR Console App (.NET Framework)", se configuró el
// subsistema como Windows para que solo se muestre la ventana de Windows Forms.
[STAThreadAttribute]
int main(array<String^>^ args)
{
    Application::EnableVisualStyles();                   // estilo visual moderno de los controles
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew ComponentesConexas::MyForm()); // abre la ventana principal
    return 0;
}
