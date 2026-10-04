#pragma once
// =====================================================================================
//  Proyecto  : Identificación de centros de salud aislados tras un sismo en Lima
//  Curso     : 1AMA0726 - Matemática Computacional (UPC)
//  Algoritmo : Componentes conexas de un grafo no dirigido mediante búsqueda en anchura (BFS)
//
//  Modelo del problema:
//    - Cada VÉRTICE (H1, H2, ..., Hn) representa un centro de salud.
//    - Cada ARISTA representa una vía que sigue transitable después del sismo.
//    - Cada COMPONENTE CONEXA es un grupo de centros que todavía pueden apoyarse por tierra.
//    - Una componente de un solo vértice es un centro de salud completamente aislado.
// =====================================================================================
using namespace System;                        // Math, Random, String
using namespace System::Collections::Generic;  // Queue<int>, List<int>
using namespace System::Drawing;               // Graphics, Pen, PointF, Color
using namespace System::Windows::Forms;        // Form, Button, DataGridView
using namespace System::Text;                  // StringBuilder

namespace ComponentesConexas {

    public ref class MyForm : public Form
    {
    public:
        MyForm()
        {
            InicializarDatos();
            InicializarInterfaz();
            PrepararTablaConexiones();
            ReiniciarAlgoritmo();
        }

    private:
        // =============================================================================
        //  DATOS DEL GRAFO Y DEL ALGORITMO
        // =============================================================================
        int n;                          // número de centros de salud (vértices)
        array<int, 2>^ matriz;          // matriz de adyacencia A (n x n): 1 = hay vía, 0 = no hay
        array<bool>^ visitado;          // visitado[i] = true si el centro i ya fue alcanzado por el BFS
        array<int>^ componente;         // componente[i] = número de la componente (1..k) del centro i
        Queue<int>^ cola;               // cola del BFS (centros descubiertos pendientes de procesar)
        int k;                          // cantidad de componentes encontradas hasta el momento
        int actual;                     // centro que se está procesando en este paso (-1 = ninguno)
        int pasoNum;                    // número de paso mostrado en el registro
        bool primeraLinea;              // controla el formato del registro dentro de un mismo paso
        bool grafoListo;                // true cuando ya se construyó el grafo
        bool finalizado;                // true cuando el algoritmo terminó
        bool actualizandoTabla;         // evita bucles al reflejar casillas en la tabla manual
        List<int>^ vecinosNuevos;       // vecinos descubiertos en el último paso (para resaltarlos)
        List<Point>^ viasDisponibles;   // aristas existentes (para el combo de colapso)
        List<Point>^ viasColapsadas;    // aristas eliminadas por la simulación de colapso
        array<Color>^ paleta;           // un color distinto por componente

        // =============================================================================
        //  CONTROLES DE LA INTERFAZ
        // =============================================================================
        NumericUpDown^ numN;
        RadioButton^ rbAleatorio;
        RadioButton^ rbManual;
        Button^ btnConstruir;
        Label^ lblAyuda;
        DataGridView^ dgvConexiones;    // tabla de casillas para el modo manual
        PictureBox^ pbGrafo;            // lienzo donde se dibuja el grafo
        DataGridView^ dgvMatriz;        // matriz de adyacencia en ceros y unos
        Button^ btnSiguiente;
        Button^ btnResolver;
        Button^ btnReiniciar;
        Label^ lblCola;
        Label^ lblComp;
        ComboBox^ cmbVias;
        Button^ btnColapso;
        ListBox^ lstRegistro;
        RichTextBox^ rtbResultado;

        // Colores y fuentes de la aplicación
        Color colFondo, colTarjeta, colPrimario, colTexto, colGris, colNoVisitado;
        Color colProcesando, colEnCola, colAlerta, colVerde;
        System::Drawing::Font^ fTitulo;
        System::Drawing::Font^ fSeccion;
        System::Drawing::Font^ fNormal;
        System::Drawing::Font^ fNodo;
        System::Drawing::Font^ fLeyenda;

        // =============================================================================
        //  INICIALIZACIÓN
        // =============================================================================
        void InicializarDatos()
        {
            n = 7;
            matriz = gcnew array<int, 2>(n, n);
            vecinosNuevos = gcnew List<int>();
            viasDisponibles = gcnew List<Point>();
            viasColapsadas = gcnew List<Point>();
            grafoListo = false;
            actualizandoTabla = false;

            colFondo = Color::FromArgb(243, 245, 249);
            colTarjeta = Color::White;
            colPrimario = Color::FromArgb(31, 58, 95);
            colTexto = Color::FromArgb(33, 37, 41);
            colGris = Color::FromArgb(108, 117, 125);
            colNoVisitado = Color::FromArgb(203, 210, 220);
            colProcesando = Color::FromArgb(255, 140, 0);
            colEnCola = Color::FromArgb(255, 193, 7);
            colAlerta = Color::FromArgb(200, 35, 51);
            colVerde = Color::FromArgb(25, 135, 84);

            // Paleta de 12 colores: como máximo puede haber 12 componentes (n <= 12)
            paleta = gcnew array<Color>{
                Color::FromArgb(52, 120, 246),  Color::FromArgb(46, 160, 67),
                Color::FromArgb(142, 68, 173),  Color::FromArgb(0, 150, 136),
                Color::FromArgb(214, 51, 132),  Color::FromArgb(141, 110, 99),
                Color::FromArgb(31, 58, 147),   Color::FromArgb(124, 179, 66),
                Color::FromArgb(194, 24, 91),   Color::FromArgb(84, 110, 122),
                Color::FromArgb(92, 107, 192),  Color::FromArgb(0, 131, 143)
            };

            fTitulo = gcnew System::Drawing::Font(L"Segoe UI Semibold", 16.0f);
            fSeccion = gcnew System::Drawing::Font(L"Segoe UI Semibold", 10.5f);
            fNormal = gcnew System::Drawing::Font(L"Segoe UI", 9.5f);
            fNodo = gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.5f);
            fLeyenda = gcnew System::Drawing::Font(L"Segoe UI", 8.5f);
        }

        // Aplica un estilo plano y moderno a un botón
        void EstilizarBoton(Button^ b, Color c)
        {
            b->FlatStyle = FlatStyle::Flat;
            b->FlatAppearance->BorderSize = 0;
            b->BackColor = c;
            b->ForeColor = Color::White;
            b->Font = gcnew System::Drawing::Font(L"Segoe UI Semibold", 10.0f);
            b->Cursor = Cursors::Hand;
        }

        // Crea una etiqueta de título de sección
        Label^ CrearTituloSeccion(String^ texto)
        {
            Label^ l = gcnew Label();
            l->Text = texto;
            l->Font = fSeccion;
            l->ForeColor = colPrimario;
            l->Dock = DockStyle::Top;
            l->Height = 28;
            return l;
        }

        // Crea una "tarjeta" blanca que contiene un control principal y su título
        Panel^ CrearTarjeta(String^ titulo, Control^ contenido)
        {
            Panel^ p = gcnew Panel();
            p->Dock = DockStyle::Fill;
            p->BackColor = colTarjeta;
            p->Padding = System::Windows::Forms::Padding(10);
            p->Margin = System::Windows::Forms::Padding(6);
            contenido->Dock = DockStyle::Fill;
            p->Controls->Add(contenido);                 // primero el control que llena
            p->Controls->Add(CrearTituloSeccion(titulo)); // luego el título arriba
            return p;
        }

        // Configuración común de las tablas (DataGridView)
        void EstilizarTabla(DataGridView^ d)
        {
            d->AllowUserToAddRows = false;
            d->AllowUserToDeleteRows = false;
            d->AllowUserToResizeRows = false;
            d->AllowUserToResizeColumns = false;
            d->BackgroundColor = Color::White;
            d->BorderStyle = System::Windows::Forms::BorderStyle::None;
            d->GridColor = Color::FromArgb(222, 226, 230);
            d->EnableHeadersVisualStyles = false;
            d->ColumnHeadersDefaultCellStyle->BackColor = colPrimario;
            d->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
            d->ColumnHeadersDefaultCellStyle->Font = gcnew System::Drawing::Font(L"Segoe UI Semibold", 8.5f);
            d->ColumnHeadersDefaultCellStyle->Alignment = DataGridViewContentAlignment::MiddleCenter;
            d->ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode::DisableResizing;
            d->ColumnHeadersHeight = 26;
            d->RowHeadersDefaultCellStyle->BackColor = colPrimario;
            d->RowHeadersDefaultCellStyle->ForeColor = Color::White;
            d->RowHeadersDefaultCellStyle->Font = gcnew System::Drawing::Font(L"Segoe UI Semibold", 8.5f);
            d->RowHeadersWidthSizeMode = DataGridViewRowHeadersWidthSizeMode::DisableResizing;
            d->RowHeadersWidth = 62;
            d->DefaultCellStyle->Alignment = DataGridViewContentAlignment::MiddleCenter;
            d->DefaultCellStyle->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.0f);
            d->DefaultCellStyle->SelectionBackColor = Color::FromArgb(220, 230, 245);
            d->DefaultCellStyle->SelectionForeColor = colTexto;
            d->RowTemplate->Height = 24;
            d->SelectionMode = DataGridViewSelectionMode::CellSelect;
            d->MultiSelect = false;
        }

        // Construye todos los controles de la ventana
        void InicializarInterfaz()
        {
            this->Text = L"Red de salud post-sismo - Componentes conexas";
            this->BackColor = colFondo;
            this->Font = fNormal;
            this->ClientSize = System::Drawing::Size(1340, 760);
            this->MinimumSize = System::Drawing::Size(1150, 700);
            this->StartPosition = FormStartPosition::CenterScreen;
            this->WindowState = FormWindowState::Maximized;

            // ---------------- Encabezado ----------------
            Panel^ encabezado = gcnew Panel();
            encabezado->Dock = DockStyle::Top;
            encabezado->Height = 66;
            encabezado->BackColor = colPrimario;
            Label^ titulo = gcnew Label();
            titulo->Text = L"Identificación de centros de salud aislados tras un sismo";
            titulo->Font = fTitulo;
            titulo->ForeColor = Color::White;
            titulo->AutoSize = true;
            titulo->Location = Point(18, 8);
            Label^ subtitulo = gcnew Label();
            subtitulo->Text = L"Matemática Computacional  ·  Componentes conexas mediante búsqueda en anchura (BFS)";
            subtitulo->ForeColor = Color::FromArgb(201, 214, 234);
            subtitulo->AutoSize = true;
            subtitulo->Location = Point(20, 40);
            encabezado->Controls->Add(titulo);
            encabezado->Controls->Add(subtitulo);

            // ---------------- Distribución principal en 3 columnas ----------------
            TableLayoutPanel^ principal = gcnew TableLayoutPanel();
            principal->Dock = DockStyle::Fill;
            principal->Padding = System::Windows::Forms::Padding(8);
            principal->ColumnCount = 3;
            principal->RowCount = 1;
            principal->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Absolute, 380.0f));
            principal->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 100.0f));
            principal->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Absolute, 370.0f));
            principal->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 100.0f));

            // =============== COLUMNA IZQUIERDA: configuración + tabla manual ===============
            TableLayoutPanel^ izq = gcnew TableLayoutPanel();
            izq->Dock = DockStyle::Fill;
            izq->Margin = System::Windows::Forms::Padding(0);
            izq->ColumnCount = 1;
            izq->RowCount = 2;
            izq->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 270.0f));
            izq->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 100.0f));

            Panel^ config = gcnew Panel();
            config->Dock = DockStyle::Fill;
            config->BackColor = colTarjeta;
            config->Margin = System::Windows::Forms::Padding(6);
            config->Padding = System::Windows::Forms::Padding(10);

            Label^ s1 = gcnew Label();
            s1->Text = L"1. Configuración de la red";
            s1->Font = fSeccion; s1->ForeColor = colPrimario;
            s1->Location = Point(12, 10); s1->AutoSize = true;

            Label^ lblN = gcnew Label();
            lblN->Text = L"Número de centros de salud (4 a 12):";
            lblN->Location = Point(14, 48); lblN->AutoSize = true;

            numN = gcnew NumericUpDown();
            numN->Minimum = Decimal(4);      // el control no permite valores fuera de [4, 12]
            numN->Maximum = Decimal(12);
            numN->Value = Decimal(7);
            numN->Location = Point(258, 45);
            numN->Width = 60;
            numN->ValueChanged += gcnew EventHandler(this, &MyForm::numN_ValueChanged);

            Label^ lblModo = gcnew Label();
            lblModo->Text = L"Modo de generación del grafo:";
            lblModo->Location = Point(14, 84); lblModo->AutoSize = true;

            rbAleatorio = gcnew RadioButton();
            rbAleatorio->Text = L"Aleatorio";
            rbAleatorio->Location = Point(18, 108); rbAleatorio->AutoSize = true;
            rbAleatorio->Checked = true;

            rbManual = gcnew RadioButton();
            rbManual->Text = L"Manual";
            rbManual->Location = Point(140, 108); rbManual->AutoSize = true;
            rbManual->CheckedChanged += gcnew EventHandler(this, &MyForm::rbManual_CheckedChanged);

            btnConstruir = gcnew Button();
            btnConstruir->Text = L"Construir grafo";
            btnConstruir->Location = Point(14, 142);
            btnConstruir->Size = System::Drawing::Size(304, 38);
            EstilizarBoton(btnConstruir, colPrimario);
            btnConstruir->Click += gcnew EventHandler(this, &MyForm::btnConstruir_Click);

            lblAyuda = gcnew Label();
            lblAyuda->ForeColor = colGris;
            lblAyuda->Font = fLeyenda;
            lblAyuda->Location = Point(14, 188);
            lblAyuda->Size = System::Drawing::Size(306, 62);

            config->Controls->Add(s1);
            config->Controls->Add(lblN);
            config->Controls->Add(numN);
            config->Controls->Add(lblModo);
            config->Controls->Add(rbAleatorio);
            config->Controls->Add(rbManual);
            config->Controls->Add(btnConstruir);
            config->Controls->Add(lblAyuda);

            dgvConexiones = gcnew DataGridView();
            EstilizarTabla(dgvConexiones);
            dgvConexiones->RowHeadersWidth = 56;   // ancho del encabezado de fila (H1..H12)
            dgvConexiones->CellBeginEdit += gcnew DataGridViewCellCancelEventHandler(this, &MyForm::dgvConexiones_CellBeginEdit);
            dgvConexiones->CurrentCellDirtyStateChanged += gcnew EventHandler(this, &MyForm::dgvConexiones_CurrentCellDirtyStateChanged);
            dgvConexiones->CellValueChanged += gcnew DataGridViewCellEventHandler(this, &MyForm::dgvConexiones_CellValueChanged);

            izq->Controls->Add(config, 0, 0);
            izq->Controls->Add(CrearTarjeta(L"Tabla de vías transitables", dgvConexiones), 0, 1);

            // =============== COLUMNA CENTRAL: grafo + matriz de adyacencia ===============
            TableLayoutPanel^ centro = gcnew TableLayoutPanel();
            centro->Dock = DockStyle::Fill;
            centro->Margin = System::Windows::Forms::Padding(0);
            centro->ColumnCount = 1;
            centro->RowCount = 2;
            centro->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 62.0f));
            centro->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 38.0f));

            pbGrafo = gcnew PictureBox();
            pbGrafo->BackColor = Color::White;
            pbGrafo->Paint += gcnew PaintEventHandler(this, &MyForm::pbGrafo_Paint);
            pbGrafo->Resize += gcnew EventHandler(this, &MyForm::pbGrafo_Resize);

            dgvMatriz = gcnew DataGridView();
            EstilizarTabla(dgvMatriz);
            dgvMatriz->ReadOnly = true;
            dgvMatriz->DefaultCellStyle->SelectionBackColor = Color::White;

            centro->Controls->Add(CrearTarjeta(L"Red de centros de salud", pbGrafo), 0, 0);
            centro->Controls->Add(CrearTarjeta(L"Matriz de adyacencia A", dgvMatriz), 0, 1);

            // =============== COLUMNA DERECHA: algoritmo, registro y resultado ===============
            TableLayoutPanel^ der = gcnew TableLayoutPanel();
            der->Dock = DockStyle::Fill;
            der->Margin = System::Windows::Forms::Padding(0);
            der->ColumnCount = 1;
            der->RowCount = 3;
            der->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 290.0f));
            der->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 55.0f));
            der->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 45.0f));

            Panel^ algo = gcnew Panel();
            algo->Dock = DockStyle::Fill;
            algo->BackColor = colTarjeta;
            algo->Margin = System::Windows::Forms::Padding(6);

            Label^ s2 = gcnew Label();
            s2->Text = L"2. Algoritmo BFS paso a paso";
            s2->Font = fSeccion; s2->ForeColor = colPrimario;
            s2->Location = Point(12, 10); s2->AutoSize = true;

            btnSiguiente = gcnew Button();
            btnSiguiente->Text = L"Siguiente paso  >";
            btnSiguiente->Location = Point(14, 42);
            btnSiguiente->Size = System::Drawing::Size(328, 40);
            EstilizarBoton(btnSiguiente, Color::FromArgb(52, 120, 246));
            btnSiguiente->Click += gcnew EventHandler(this, &MyForm::btnSiguiente_Click);

            btnResolver = gcnew Button();
            btnResolver->Text = L"Resolver todo";
            btnResolver->Location = Point(14, 88);
            btnResolver->Size = System::Drawing::Size(160, 34);
            EstilizarBoton(btnResolver, Color::FromArgb(0, 150, 136));
            btnResolver->Click += gcnew EventHandler(this, &MyForm::btnResolver_Click);

            btnReiniciar = gcnew Button();
            btnReiniciar->Text = L"Reiniciar";
            btnReiniciar->Location = Point(182, 88);
            btnReiniciar->Size = System::Drawing::Size(160, 34);
            EstilizarBoton(btnReiniciar, colGris);
            btnReiniciar->Click += gcnew EventHandler(this, &MyForm::btnReiniciar_Click);

            lblCola = gcnew Label();
            lblCola->Location = Point(14, 132);
            lblCola->Size = System::Drawing::Size(330, 22);

            lblComp = gcnew Label();
            lblComp->Location = Point(14, 156);
            lblComp->Size = System::Drawing::Size(330, 22);

            Label^ s3 = gcnew Label();
            s3->Text = L"3. Simular colapso de una vía";
            s3->Font = fSeccion; s3->ForeColor = colPrimario;
            s3->Location = Point(12, 190); s3->AutoSize = true;

            cmbVias = gcnew ComboBox();
            cmbVias->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbVias->Location = Point(14, 226);
            cmbVias->Width = 130;

            btnColapso = gcnew Button();
            btnColapso->Text = L"Simular colapso de vía";
            btnColapso->Location = Point(152, 222);
            btnColapso->Size = System::Drawing::Size(190, 34);
            EstilizarBoton(btnColapso, colAlerta);
            btnColapso->Click += gcnew EventHandler(this, &MyForm::btnColapso_Click);

            algo->Controls->Add(s2);
            algo->Controls->Add(btnSiguiente);
            algo->Controls->Add(btnResolver);
            algo->Controls->Add(btnReiniciar);
            algo->Controls->Add(lblCola);
            algo->Controls->Add(lblComp);
            algo->Controls->Add(s3);
            algo->Controls->Add(cmbVias);
            algo->Controls->Add(btnColapso);

            lstRegistro = gcnew ListBox();
            lstRegistro->BorderStyle = System::Windows::Forms::BorderStyle::None;
            lstRegistro->HorizontalScrollbar = true;
            lstRegistro->IntegralHeight = false;

            rtbResultado = gcnew RichTextBox();
            rtbResultado->BorderStyle = System::Windows::Forms::BorderStyle::None;
            rtbResultado->ReadOnly = true;
            rtbResultado->BackColor = Color::White;
            rtbResultado->Font = gcnew System::Drawing::Font(L"Segoe UI", 10.0f);

            der->Controls->Add(algo, 0, 0);
            der->Controls->Add(CrearTarjeta(L"Registro de pasos", lstRegistro), 0, 1);
            der->Controls->Add(CrearTarjeta(L"Resultado", rtbResultado), 0, 2);

            principal->Controls->Add(izq, 0, 0);
            principal->Controls->Add(centro, 1, 0);
            principal->Controls->Add(der, 2, 0);

            // Orden de acoplamiento: primero el contenido que llena, luego el encabezado
            this->Controls->Add(principal);
            this->Controls->Add(encabezado);

            ActualizarAyuda();
            HabilitarControlesAlgoritmo(false);
        }

        // =============================================================================
        //  TABLA DE VÍAS (MODO MANUAL)
        // =============================================================================

        // Crea una tabla n x n de casillas. Solo la parte superior (j > i) es editable,
        // porque el grafo es no dirigido: marcar (i, j) equivale a marcar (j, i).
        void PrepararTablaConexiones()
        {
            actualizandoTabla = true;
            dgvConexiones->Rows->Clear();
            dgvConexiones->Columns->Clear();
            for (int j = 0; j < n; j++) {
                DataGridViewCheckBoxColumn^ col = gcnew DataGridViewCheckBoxColumn();
                col->HeaderText = String::Format(L"H{0}", j + 1);
                col->Width = 22;
                col->SortMode = DataGridViewColumnSortMode::NotSortable;
                dgvConexiones->Columns->Add(col);
            }
            dgvConexiones->RowCount = n;
            for (int i = 0; i < n; i++) {
                dgvConexiones->Rows[i]->HeaderCell->Value = String::Format(L"H{0}", i + 1);
                for (int j = 0; j < n; j++) {
                    dgvConexiones->Rows[i]->Cells[j]->Value = false;
                    if (j <= i)   // diagonal y parte inferior: solo se muestran (en gris)
                        dgvConexiones->Rows[i]->Cells[j]->Style->BackColor = Color::FromArgb(233, 236, 241);
                }
            }
            actualizandoTabla = false;
        }

        // Copia la matriz de adyacencia en la tabla de casillas (útil en modo aleatorio y tras un colapso)
        void ActualizarTablaDesdeMatriz()
        {
            actualizandoTabla = true;
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    dgvConexiones->Rows[i]->Cells[j]->Value = (matriz[i, j] == 1);
            actualizandoTabla = false;
        }

        // Impide editar la diagonal, la parte inferior o cualquier casilla en modo aleatorio
        void dgvConexiones_CellBeginEdit(Object^ sender, DataGridViewCellCancelEventArgs^ e)
        {
            if (!rbManual->Checked || e->ColumnIndex <= e->RowIndex)
                e->Cancel = true;
        }

        // Confirma el clic en una casilla de inmediato (sin esperar a cambiar de celda)
        void dgvConexiones_CurrentCellDirtyStateChanged(Object^ sender, EventArgs^ e)
        {
            if (dgvConexiones->IsCurrentCellDirty)
                dgvConexiones->CommitEdit(DataGridViewDataErrorContexts::Commit);
        }

        // Al marcar (i, j) se refleja automáticamente en (j, i) para mostrar la simetría
        void dgvConexiones_CellValueChanged(Object^ sender, DataGridViewCellEventArgs^ e)
        {
            if (actualizandoTabla || e->RowIndex < 0 || e->ColumnIndex < 0) return;
            int i = e->RowIndex, j = e->ColumnIndex;
            if (j <= i) return;
            actualizandoTabla = true;
            dgvConexiones->Rows[j]->Cells[i]->Value = dgvConexiones->Rows[i]->Cells[j]->Value;
            actualizandoTabla = false;
        }

        // =============================================================================
        //  CONSTRUCCIÓN DEL GRAFO
        // =============================================================================
        void ConstruirGrafo()
        {
            n = Decimal::ToInt32(numN->Value);
            if (n < 4 || n > 12) {   // validación del rango solicitado por el proyecto
                MessageBox::Show(L"El número de centros de salud debe estar entre 4 y 12.",
                    L"Dato no válido", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (dgvConexiones->RowCount != n) PrepararTablaConexiones();

            if (rbAleatorio->Checked) {
                // ------------------------------------------------------------------
                // MODO ALEATORIO: se recorre solo la parte superior de la matriz
                // (j > i) y cada posible vía se crea con probabilidad p. Al asignar
                // el mismo valor en [i, j] y [j, i] la matriz queda simétrica,
                // como corresponde a un grafo NO dirigido.
                // ------------------------------------------------------------------
                matriz = gcnew array<int, 2>(n, n);         // se inicializa con ceros
                Random^ azar = gcnew Random();
                double p = 0.25;
                for (int i = 0; i < n; i++)
                    for (int j = i + 1; j < n; j++)
                        if (azar->NextDouble() < p) {
                            matriz[i, j] = 1;
                            matriz[j, i] = 1;                   // simetria: grafo no dirigido
                        }
                ActualizarTablaDesdeMatriz();
            }
            else {
                // ------------------------------------------------------------------
                // MODO MANUAL: se leen las casillas marcadas por el usuario en la
                // tabla de vías. Una casilla vacía (null) se interpreta como false.
                // ------------------------------------------------------------------
                matriz = gcnew array<int, 2>(n, n);
                for (int i = 0; i < n; i++)
                    for (int j = i + 1; j < n; j++) {
                        Object^ valor = dgvConexiones->Rows[i]->Cells[j]->Value;
                        bool marcada = Convert::ToBoolean(valor);   // casilla vacia = false
                        matriz[i, j] = marcada ? 1 : 0;
                        matriz[j, i] = matriz[i, j];
                    }
            }

            viasColapsadas->Clear();
            grafoListo = true;
            MostrarMatriz();
            LlenarComboVias();
            ReiniciarAlgoritmo();
            HabilitarControlesAlgoritmo(true);
            Anotar(String::Format(L"Grafo construido: {0} centros de salud y {1} vías transitables.",
                n, ContarAristas()));
            Anotar(L"Pulse «Siguiente paso» para iniciar el algoritmo.");
        }

        int ContarAristas()
        {
            int m = 0;
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    m += matriz[i, j];
            return m;
        }

        // Muestra la matriz de adyacencia (0/1) y el grado de cada vértice (suma de su fila)
        void MostrarMatriz()
        {
            dgvMatriz->Rows->Clear();
            dgvMatriz->Columns->Clear();
            for (int j = 0; j < n; j++) {
                DataGridViewTextBoxColumn^ col = gcnew DataGridViewTextBoxColumn();
                col->HeaderText = String::Format(L"H{0}", j + 1);
                col->Width = 36;
                col->SortMode = DataGridViewColumnSortMode::NotSortable;
                dgvMatriz->Columns->Add(col);
            }
            DataGridViewTextBoxColumn^ colGrado = gcnew DataGridViewTextBoxColumn();
            colGrado->HeaderText = L"Grado";
            colGrado->Width = 56;
            colGrado->SortMode = DataGridViewColumnSortMode::NotSortable;
            dgvMatriz->Columns->Add(colGrado);

            dgvMatriz->RowCount = n;
            for (int i = 0; i < n; i++) {
                dgvMatriz->Rows[i]->HeaderCell->Value = String::Format(L"H{0}", i + 1);
                int grado = 0;
                for (int j = 0; j < n; j++) {
                    dgvMatriz->Rows[i]->Cells[j]->Value = matriz[i, j];
                    grado += matriz[i, j];            // grado = suma de la fila i
                }
                dgvMatriz->Rows[i]->Cells[n]->Value = grado;
                dgvMatriz->Rows[i]->Cells[n]->Style->Font = fNodo;
            }
            ResaltarMatriz();
        }

        // Pinta la fila del centro que se procesa (naranja claro) y las casillas de los
        // vecinos descubiertos en este paso (verde claro). Las vías colapsadas se ven en rojo.
        void ResaltarMatriz()
        {
            if (!grafoListo || dgvMatriz->RowCount != n) return;
            for (int i = 0; i < n; i++)
                for (int j = 0; j <= n; j++) {
                    DataGridViewCell^ celda = dgvMatriz->Rows[i]->Cells[j];
                    Color fondo = Color::White;
                    if (i == actual) fondo = Color::FromArgb(255, 232, 204);
                    if (i == actual && j < n && vecinosNuevos->Contains(j)) fondo = Color::FromArgb(204, 237, 214);
                    celda->Style->BackColor = fondo;
                    celda->Style->SelectionBackColor = fondo;
                    celda->Style->ForeColor = colTexto;
                    if (j < n && EsViaColapsada(i, j)) celda->Style->ForeColor = colAlerta;
                }
            dgvMatriz->ClearSelection();
        }

        bool EsViaColapsada(int i, int j)
        {
            for each(Point v in viasColapsadas)
                if ((v.X == i && v.Y == j) || (v.X == j && v.Y == i)) return true;
            return false;
        }

        // Llena el combo con las vías existentes para poder simular su colapso
        void LlenarComboVias()
        {
            cmbVias->Items->Clear();
            viasDisponibles->Clear();
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    if (matriz[i, j] == 1) {
                        viasDisponibles->Add(Point(i, j));
                        cmbVias->Items->Add(String::Format(L"H{0} - H{1}", i + 1, j + 1));
                    }
            if (cmbVias->Items->Count > 0) cmbVias->SelectedIndex = 0;
            btnColapso->Enabled = grafoListo && cmbVias->Items->Count > 0;
        }

        // =============================================================================
        //  ALGORITMO DE COMPONENTES CONEXAS (BFS PASO A PASO)
        // =============================================================================

        // Etapa 1 - Inicialización: ningún centro visitado, cola vacía y k = 0
        void ReiniciarAlgoritmo()
        {
            visitado = gcnew array<bool>(n);
            componente = gcnew array<int>(n);
            cola = gcnew Queue<int>();
            vecinosNuevos->Clear();
            k = 0;
            actual = -1;
            pasoNum = 0;
            finalizado = false;
            lstRegistro->Items->Clear();
            rtbResultado->Clear();
            rtbResultado->SelectionColor = colGris;
            if (grafoListo)
                rtbResultado->AppendText(L"El resultado aparecerá aquí cuando el algoritmo termine.");
            else
                rtbResultado->AppendText(L"Configure la red y pulse «Construir grafo».");
            btnSiguiente->Enabled = grafoListo;
            btnResolver->Enabled = grafoListo;
            ActualizarVista();
        }

        // Etapa 2 (apoyo): devuelve el primer centro no visitado, o -1 si ya no quedan
        int BuscarNoVisitado()
        {
            for (int i = 0; i < n; i++)
                if (!visitado[i]) return i;
            return -1;
        }

        // ---------------------------------------------------------------------------------
        //  Ejecuta UNA sola acción del BFS en cada clic de "Siguiente paso":
        //   * Si la cola está vacía, se inicia una nueva componente desde el primer
        //     centro no visitado (Etapa 2). Si ya no quedan, se muestra el resultado (Etapa 5).
        //   * Si la cola tiene elementos, se extrae el centro del frente y se agregan a la
        //     cola todos sus vecinos no visitados (Etapa 3). Cuando la cola se vacía,
        //     la componente actual queda cerrada (Etapa 4).
        // ---------------------------------------------------------------------------------
        void SiguientePaso()
        {
            if (!grafoListo || finalizado) return;
            pasoNum++;
            primeraLinea = true;
            vecinosNuevos->Clear();

            if (cola->Count == 0) {
                int s = BuscarNoVisitado();            // devuelve -1 si no quedan
                if (s == -1) { MostrarResultado(); return; }
                k++;                                   // nueva componente C_k
                visitado[s] = true;
                componente[s] = k;
                cola->Enqueue(s);
                actual = -1;
                Registrar(String::Format(L"Se inicia C{0} desde H{1}; H{1} ingresa a la cola.", k, s + 1));
            }
            else {
                int u = cola->Dequeue();               // vertice que se procesa
                actual = u;
                Registrar(String::Format(L"Se extrae H{0} de la cola y se revisa su fila en A.", u + 1));


                for (int v = 0; v < n; v++)
                    if (matriz[u, v] == 1 && !visitado[v]) {
                        visitado[v] = true;
                        componente[v] = k;
                        cola->Enqueue(v);
                        vecinosNuevos->Add(v);
                        Registrar(String::Format(L"Desde H{0} se alcanza H{1}.", u + 1, v + 1));
                    }

                if (vecinosNuevos->Count == 0)
                    Registrar(String::Format(L"H{0} no tiene vecinos sin visitar.", u + 1));

    
                if (cola->Count == 0)
                    Registrar(String::Format(L"La cola quedó vacía: la componente C{0} se cierra.", k));
            }
            ActualizarVista();
        }

        // Etapa 5 - Resultado: número de componentes, sus centros y los centros aislados
        void MostrarResultado()
        {
            finalizado = true;
            actual = -1;
            Registrar(L"No quedan centros sin visitar; el algoritmo termina.");

            // Se agrupan los centros de cada componente en listas
            array<List<int>^>^ grupos = gcnew array<List<int>^>(k);
            for (int c = 0; c < k; c++) grupos[c] = gcnew List<int>();
            for (int i = 0; i < n; i++) grupos[componente[i] - 1]->Add(i);

            rtbResultado->Clear();
            EscribirResultado(L"Componentes conexas encontradas: ", colTexto, false);
            EscribirResultado(String::Format(L"{0}\n\n", k), colPrimario, true);

            List<int>^ aislados = gcnew List<int>();
            for (int c = 0; c < k; c++) {
                StringBuilder^ sb = gcnew StringBuilder();
                for (int t = 0; t < grupos[c]->Count; t++) {
                    if (t > 0) sb->Append(L", ");
                    sb->Append(String::Format(L"H{0}", grupos[c][t] + 1));
                }
                EscribirResultado(String::Format(L"C{0}", c + 1), paleta[c % paleta->Length], true);
                String^ plural = (grupos[c]->Count == 1) ? String::Empty : gcnew String(L"s");
                EscribirResultado(String::Format(L" = {{{0}}}  ({1} centro{2})\n", sb->ToString(),
                    grupos[c]->Count, plural), colTexto, false);
                if (grupos[c]->Count == 1) aislados->Add(grupos[c][0]);
            }

            // Interpretación del resultado en el contexto del problema
            EscribirResultado(L"\n", colTexto, false);
            if (k == 1) {
                EscribirResultado(L"La red está totalmente comunicada: todos los centros de salud "
                    L"pueden apoyarse por tierra.\n", colVerde, true);
            }
            else {
                EscribirResultado(String::Format(L"La red quedó dividida en {0} grupos incomunicados entre sí. "
                    L"Cada grupo debe coordinar su propia atención.\n", k), colTexto, false);
            }
            if (aislados->Count > 0) {
                StringBuilder^ sb = gcnew StringBuilder();
                for (int t = 0; t < aislados->Count; t++) {
                    if (t > 0) sb->Append(L", ");
                    sb->Append(String::Format(L"H{0}", aislados[t] + 1));
                }
                EscribirResultado(String::Format(L"\nALERTA: centros completamente aislados: {0}. "
                    L"Requieren atención prioritaria por vía aérea o brigadas.\n", sb->ToString()),
                    colAlerta, true);
            }

            btnSiguiente->Enabled = false;
            btnResolver->Enabled = false;
            ActualizarVista();
        }

        void EscribirResultado(String^ texto, Color color, bool negrita)
        {
            rtbResultado->SelectionStart = rtbResultado->TextLength;
            rtbResultado->SelectionLength = 0;
            rtbResultado->SelectionColor = color;
            rtbResultado->SelectionFont = gcnew System::Drawing::Font(L"Segoe UI", 10.0f,
                negrita ? FontStyle::Bold : FontStyle::Regular);
            rtbResultado->AppendText(texto);
        }

        // Agrega una línea al registro. La primera línea de cada paso lleva el número de paso.
        void Registrar(String^ texto)
        {
            if (primeraLinea) {
                lstRegistro->Items->Add(String::Format(L"Paso {0}: {1}", pasoNum, texto));
                primeraLinea = false;
            }
            else {
                lstRegistro->Items->Add(L"        " + texto);
            }
            lstRegistro->TopIndex = lstRegistro->Items->Count - 1;   // desplaza a la última línea
        }

        // Nota informativa en el registro (no cuenta como paso del algoritmo)
        void Anotar(String^ texto)
        {
            lstRegistro->Items->Add(L"» " + texto);
            lstRegistro->TopIndex = lstRegistro->Items->Count - 1;
        }

        // Actualiza etiquetas, resaltado de la matriz y el dibujo
        void ActualizarVista()
        {
            if (cola != nullptr) {
                StringBuilder^ sb = gcnew StringBuilder(L"Cola: [");
                array<int>^ elementos = cola->ToArray();
                for (int t = 0; t < elementos->Length; t++) {
                    if (t > 0) sb->Append(L", ");
                    sb->Append(String::Format(L"H{0}", elementos[t] + 1));
                }
                sb->Append(L"]");
                lblCola->Text = sb->ToString();
            }
            lblComp->Text = String::Format(L"Componentes encontradas (k): {0}", k);
            ResaltarMatriz();
            pbGrafo->Invalidate();
        }

        void HabilitarControlesAlgoritmo(bool activo)
        {
            btnSiguiente->Enabled = activo;
            btnResolver->Enabled = activo;
            btnReiniciar->Enabled = activo;
            btnColapso->Enabled = activo && cmbVias->Items->Count > 0;
            cmbVias->Enabled = activo;
        }

        void ActualizarAyuda()
        {
            if (rbManual->Checked)
                lblAyuda->Text = L"Modo manual: marque en la tabla las vías que siguen transitables "
                L"(parte blanca) y pulse «Construir grafo».";
            else
                lblAyuda->Text = L"Modo aleatorio: cada vía posible existe con una probabilidad "
                L"de 35 %. Pulse «Construir grafo» para generar la red.";
        }

        // =============================================================================
        //  DIBUJO DEL GRAFO
        // =============================================================================
        void pbGrafo_Paint(Object^ sender, PaintEventArgs^ e)
        {
            Graphics^ g = e->Graphics;
            g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
            g->TextRenderingHint = System::Drawing::Text::TextRenderingHint::ClearTypeGridFit;
            g->Clear(Color::White);

            StringFormat^ centrado = gcnew StringFormat();
            centrado->Alignment = StringAlignment::Center;
            centrado->LineAlignment = StringAlignment::Center;

            if (!grafoListo) {
                SolidBrush^ bt = gcnew SolidBrush(colGris);
                g->DrawString(L"Configure la red y pulse «Construir grafo»", fNormal, bt,
                    RectangleF(0, 0, (float)pbGrafo->Width, (float)pbGrafo->Height), centrado);
                delete bt;
                return;
            }

            // ---------------------------------------------------------------------
            // Ubicación de los n vértices sobre una circunferencia de centro (cx, cy)
            // y radio r: el vértice i se ubica en el ángulo 2*pi*i/n.
            // ---------------------------------------------------------------------
            array<PointF>^ pos = gcnew array<PointF>(n);
            float cx = pbGrafo->Width / 2.0f;
            float cy = pbGrafo->Height / 2.0f;
            float r = Math::Min(cx, cy) - 40.0f;
            if (r < 40.0f) r = 40.0f;
            for (int i = 0; i < n; i++) {
                double ang = 2.0 * Math::PI * i / n - Math::PI / 2.0;   
                pos[i] = PointF(cx + r * (float)Math::Cos(ang),
                    cy + r * (float)Math::Sin(ang));
            }
            float rn = Math::Max(15.0f, Math::Min(24.0f, r * 0.14f));  

            Pen^ lapizColapso = gcnew Pen(colAlerta, 2.0f);
            lapizColapso->DashStyle = System::Drawing::Drawing2D::DashStyle::Dash;
            for each(Point v in viasColapsadas)
                g->DrawLine(lapizColapso, pos[v.X], pos[v.Y]);

            Pen^ lapiz = gcnew Pen(Color::FromArgb(154, 165, 181), 2.0f);
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    if (matriz[i, j] == 1) {
                        if (componente[i] != 0 && componente[i] == componente[j]) {
                            Pen^ lc = gcnew Pen(paleta[(componente[i] - 1) % paleta->Length], 3.5f);
                            g->DrawLine(lc, pos[i], pos[j]);
                            delete lc;
                        }
                        else {
                            g->DrawLine(lapiz, pos[i], pos[j]);
                        }
                    }

            // Tamaño de cada componente (para marcar los centros aislados al final)
            array<int>^ tam = gcnew array<int>(k + 1);
            for (int i = 0; i < n; i++) tam[componente[i]]++;

            // Vértices: gris = no visitado; color de su componente = ya asignado;
            // borde dorado = en cola; borde naranja grueso = se está procesando.
            for (int i = 0; i < n; i++) {
                bool asignado = componente[i] != 0;
                Color relleno = asignado ? paleta[(componente[i] - 1) % paleta->Length] : colNoVisitado;
                float radio = (i == actual) ? rn + 4.0f : rn;
                RectangleF rect = RectangleF(pos[i].X - radio, pos[i].Y - radio, 2 * radio, 2 * radio);

                SolidBrush^ br = gcnew SolidBrush(relleno);
                g->FillEllipse(br, rect);
                delete br;

                Pen^ borde;
                if (i == actual)             borde = gcnew Pen(colProcesando, 5.0f);
                else if (cola->Contains(i))  borde = gcnew Pen(colEnCola, 4.0f);
                else                         borde = gcnew Pen(Color::FromArgb(70, 80, 95), 1.5f);
                g->DrawEllipse(borde, rect);
                delete borde;

                SolidBrush^ bt = gcnew SolidBrush(asignado ? Color::White : colTexto);
                g->DrawString(String::Format(L"H{0}", i + 1), fNodo, bt, rect, centrado);
                delete bt;

                if (finalizado && tam[componente[i]] == 1) {
                    SolidBrush^ ba = gcnew SolidBrush(colAlerta);
                    g->DrawString(L"AISLADO", fLeyenda, ba,
                        RectangleF(pos[i].X - 40, pos[i].Y + radio + 2, 80, 16), centrado);
                    delete ba;
                }
            }

            DibujarLeyenda(g);
            delete lapiz;
            delete lapizColapso;
        }

        // Leyenda en la esquina inferior izquierda del lienzo
        void DibujarLeyenda(Graphics^ g)
        {
            float x = 10.0f;
            float y = pbGrafo->Height - 62.0f;
            SolidBrush^ bt = gcnew SolidBrush(colGris);

            DibujarMuestra(g, x, y, colNoVisitado, Color::FromArgb(70, 80, 95), 1.5f);
            g->DrawString(L"No visitado", fLeyenda, bt, x + 18, y - 1);
            DibujarMuestra(g, x + 100, y, Color::White, colEnCola, 3.0f);
            g->DrawString(L"En cola", fLeyenda, bt, x + 118, y - 1);
            DibujarMuestra(g, x + 180, y, Color::White, colProcesando, 3.5f);
            g->DrawString(L"Procesando", fLeyenda, bt, x + 198, y - 1);

            if (viasColapsadas->Count > 0) {
                Pen^ pc = gcnew Pen(colAlerta, 2.0f);
                pc->DashStyle = System::Drawing::Drawing2D::DashStyle::Dash;
                g->DrawLine(pc, x + 290, y + 7, x + 312, y + 7);
                g->DrawString(L"Vía colapsada", fLeyenda, bt, x + 316, y - 1);
                delete pc;
            }

            // Colores de las componentes encontradas
            float yc = y + 24.0f;
            for (int c = 0; c < k; c++) {
                float xc = x + c * 58.0f;
                DibujarMuestra(g, xc, yc, paleta[c % paleta->Length], paleta[c % paleta->Length], 1.0f);
                g->DrawString(String::Format(L"C{0}", c + 1), fLeyenda, bt, xc + 18, yc - 1);
            }
            delete bt;
        }

        void DibujarMuestra(Graphics^ g, float x, float y, Color relleno, Color borde, float grosor)
        {
            SolidBrush^ b = gcnew SolidBrush(relleno);
            Pen^ p = gcnew Pen(borde, grosor);
            g->FillEllipse(b, x, y, 14.0f, 14.0f);
            g->DrawEllipse(p, x, y, 14.0f, 14.0f);
            delete b;
            delete p;
        }

        void pbGrafo_Resize(Object^ sender, EventArgs^ e)
        {
            pbGrafo->Invalidate();
        }

        // =============================================================================
        //  EVENTOS DE LOS BOTONES Y CONTROLES
        // =============================================================================
        void btnConstruir_Click(Object^ sender, EventArgs^ e)
        {
            ConstruirGrafo();
        }

        void btnSiguiente_Click(Object^ sender, EventArgs^ e)
        {
            SiguientePaso();
        }

        // Ejecuta todos los pasos restantes hasta terminar el algoritmo
        void btnResolver_Click(Object^ sender, EventArgs^ e)
        {
            while (grafoListo && !finalizado)
                SiguientePaso();
        }

        void btnReiniciar_Click(Object^ sender, EventArgs^ e)
        {
            ReiniciarAlgoritmo();
            Anotar(L"Algoritmo reiniciado sobre la misma red.");
        }

        // ---------------------------------------------------------------------------------
        //  Simulación de colapso: se elimina la arista elegida (a[i][j] = a[j][i] = 0),
        //  se actualizan la matriz y el dibujo, y se reinicia el algoritmo para comparar
        //  cómo cambian las componentes conexas después de perder esa vía.
        // ---------------------------------------------------------------------------------
        void btnColapso_Click(Object^ sender, EventArgs^ e)
        {
            int idx = cmbVias->SelectedIndex;
            if (idx < 0 || idx >= viasDisponibles->Count) return;
            Point via = viasDisponibles[idx];
            matriz[via.X, via.Y] = 0;
            matriz[via.Y, via.X] = 0;
            viasColapsadas->Add(via);

            ActualizarTablaDesdeMatriz();
            MostrarMatriz();
            LlenarComboVias();
            ReiniciarAlgoritmo();
            Anotar(String::Format(L"Colapsó la vía H{0} - H{1}. Vías transitables restantes: {2}.",
                via.X + 1, via.Y + 1, ContarAristas()));
            Anotar(L"Pulse «Siguiente paso» o «Resolver todo» para analizar la nueva red.");
        }

        // Al cambiar n se prepara una tabla nueva y se descarta el grafo anterior
        void numN_ValueChanged(Object^ sender, EventArgs^ e)
        {
            n = Decimal::ToInt32(numN->Value);
            grafoListo = false;
            PrepararTablaConexiones();
            dgvMatriz->Rows->Clear();
            dgvMatriz->Columns->Clear();
            cmbVias->Items->Clear();
            viasColapsadas->Clear();
            ReiniciarAlgoritmo();
            HabilitarControlesAlgoritmo(false);
        }

        void rbManual_CheckedChanged(Object^ sender, EventArgs^ e)
        {
            ActualizarAyuda();
        }
    };
}