import React, { useState, useMemo } from 'react';
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select";
import { Badge } from "@/components/ui/badge";
import { Calculator, BarChart3, TrendingUp, Users, Info, ChevronRight, Sparkles } from 'lucide-react';
import { 
  BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, 
  LineChart, Line, AreaChart, Area 
} from 'recharts';

// Funciones estadísticas auxiliares
const calcMedia = (data) => data.reduce((a, b) => a + b, 0) / data.length;
const calcVarianza = (data, media) => data.reduce((sum, x) => sum + Math.pow(x - media, 2), 0) / (data.length - 1);
const calcDesviacion = (varianza) => Math.sqrt(varianza);

// Función para obtener valor Z según nivel de confianza
const getZValue = (confianza) => {
  const zValues = { '0.90': 1.645, '0.95': 1.96, '0.99': 2.576 };
  return zValues[confianza] || 1.96;
};

// Función para obtener valor t (aproximado)
const getTValue = (confianza, gl) => {
  // Valores aproximados para gl comunes
  const tTable = {
    '0.90': { 10: 1.812, 20: 1.725, 30: 1.697, 50: 1.676, 100: 1.660, 1000: 1.646 },
    '0.95': { 10: 2.228, 20: 2.086, 30: 2.042, 50: 2.009, 100: 1.984, 1000: 1.962 },
    '0.99': { 10: 3.169, 20: 2.845, 30: 2.750, 50: 2.678, 100: 2.626, 1000: 2.581 }
  };
  const gls = [10, 20, 30, 50, 100, 1000];
  const closest = gls.reduce((prev, curr) => Math.abs(curr - gl) < Math.abs(prev - gl) ? curr : prev);
  return tTable[confianza]?.[closest] || 1.96;
};

// Función para calcular probabilidad normal estándar (aproximación)
const normalCDF = (z) => {
  const a1 = 0.254829592, a2 = -0.284496736, a3 = 1.421413741, a4 = -1.453152027, a5 = 1.061405429;
  const p = 0.3275911;
  const sign = z < 0 ? -1 : 1;
  z = Math.abs(z) / Math.sqrt(2);
  const t = 1.0 / (1.0 + p * z);
  const y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * Math.exp(-z * z);
  return 0.5 * (1.0 + sign * y);
};

export default function CalculadoraEstadistica() {
  // Estados principales
  const [datos, setDatos] = useState([]);
  const [inputDatos, setInputDatos] = useState('');
  const [mensaje, setMensaje] = useState({ tipo: '', texto: '' });
  
  // Estados para inferencia
  const [nivelConfianza, setNivelConfianza] = useState('0.95');
  const [proporcion, setProporcion] = useState('');
  const [tamanoMuestra, setTamanoMuestra] = useState('');
  
  // Estados para tamaño de muestra
  const [errorDeseado, setErrorDeseado] = useState('');
  const [desviacionPoblacional, setDesviacionPoblacional] = useState('');
  const [proporcionEstimada, setProporcionEstimada] = useState('0.5');
  
  // Estados para dos poblaciones
  const [datos2, setDatos2] = useState([]);
  const [inputDatos2, setInputDatos2] = useState('');
  const [prop1, setProp1] = useState({ exitos: '', n: '' });
  const [prop2, setProp2] = useState({ exitos: '', n: '' });
  const [hipotesisAlternativa, setHipotesisAlternativa] = useState('diferente');
  
  // Estados para TLC
  const [tlcN, setTlcN] = useState(30);
  const [tlcSimulaciones, setTlcSimulaciones] = useState(1000);
  const [tlcData, setTlcData] = useState([]);

  // Cargar datos principales
  const cargarDatos = () => {
    try {
      const parsed = inputDatos.split(',').map(x => parseFloat(x.trim())).filter(x => !isNaN(x));
      if (parsed.length === 0) throw new Error('No hay datos válidos');
      setDatos(parsed);
      setMensaje({ tipo: 'success', texto: `✓ ${parsed.length} datos cargados correctamente` });
    } catch (e) {
      setMensaje({ tipo: 'error', texto: '✗ Error: revisa el formato de los datos' });
    }
  };

  const cargarDatos2 = () => {
    try {
      const parsed = inputDatos2.split(',').map(x => parseFloat(x.trim())).filter(x => !isNaN(x));
      if (parsed.length === 0) throw new Error('No hay datos válidos');
      setDatos2(parsed);
      setMensaje({ tipo: 'success', texto: `✓ ${parsed.length} datos de población 2 cargados` });
    } catch (e) {
      setMensaje({ tipo: 'error', texto: '✗ Error en datos de población 2' });
    }
  };

  // Cálculos estadísticos
  const estadisticos = useMemo(() => {
    if (datos.length === 0) return null;
    const media = calcMedia(datos);
    const varianza = calcVarianza(datos, media);
    const desviacion = calcDesviacion(varianza);
    const ordenados = [...datos].sort((a, b) => a - b);
    const n = datos.length;
    const mediana = n % 2 === 0 
      ? (ordenados[n/2 - 1] + ordenados[n/2]) / 2 
      : ordenados[Math.floor(n/2)];
    const minimo = ordenados[0];
    const maximo = ordenados[n - 1];
    const rango = maximo - minimo;
    const errorEstandar = desviacion / Math.sqrt(n);
    
    // Moda
    const frecuencias = {};
    datos.forEach(x => frecuencias[x] = (frecuencias[x] || 0) + 1);
    const maxFreq = Math.max(...Object.values(frecuencias));
    const modas = Object.keys(frecuencias).filter(k => frecuencias[k] === maxFreq).map(Number);
    
    return { media, varianza, desviacion, mediana, minimo, maximo, rango, errorEstandar, n, modas, ordenados };
  }, [datos]);

  // Histograma
  const histogramaData = useMemo(() => {
    if (!estadisticos) return [];
    const { ordenados, minimo, maximo } = estadisticos;
    const numBins = Math.min(10, Math.ceil(Math.sqrt(ordenados.length)));
    const binWidth = (maximo - minimo) / numBins || 1;
    const bins = Array(numBins).fill(0);
    ordenados.forEach(x => {
      const idx = Math.min(Math.floor((x - minimo) / binWidth), numBins - 1);
      bins[idx]++;
    });
    return bins.map((count, i) => ({
      rango: `${(minimo + i * binWidth).toFixed(1)}-${(minimo + (i + 1) * binWidth).toFixed(1)}`,
      frecuencia: count
    }));
  }, [estadisticos]);

  // Intervalos de confianza
  const calcularICMedia = () => {
    if (!estadisticos) return null;
    const { media, desviacion, n } = estadisticos;
    const z = getZValue(nivelConfianza);
    const t = getTValue(nivelConfianza, n - 1);
    const errorZ = z * (desviacion / Math.sqrt(n));
    const errorT = t * (desviacion / Math.sqrt(n));
    return {
      icZ: { inferior: media - errorZ, superior: media + errorZ, error: errorZ },
      icT: { inferior: media - errorT, superior: media + errorT, error: errorT }
    };
  };

  const calcularICProporcion = () => {
    const p = parseFloat(proporcion);
    const n = parseFloat(tamanoMuestra);
    if (isNaN(p) || isNaN(n) || n <= 0 || p < 0 || p > 1) return null;
    const z = getZValue(nivelConfianza);
    const error = z * Math.sqrt((p * (1 - p)) / n);
    return { inferior: Math.max(0, p - error), superior: Math.min(1, p + error), error };
  };

  // Tamaño de muestra
  const calcularTamanoMuestraMedia = () => {
    const E = parseFloat(errorDeseado);
    const sigma = parseFloat(desviacionPoblacional);
    if (isNaN(E) || isNaN(sigma) || E <= 0 || sigma <= 0) return null;
    const z = getZValue(nivelConfianza);
    return Math.ceil(Math.pow((z * sigma) / E, 2));
  };

  const calcularTamanoMuestraProporcion = () => {
    const E = parseFloat(errorDeseado);
    const p = parseFloat(proporcionEstimada);
    if (isNaN(E) || E <= 0 || isNaN(p) || p < 0 || p > 1) return null;
    const z = getZValue(nivelConfianza);
    return Math.ceil((Math.pow(z, 2) * p * (1 - p)) / Math.pow(E, 2));
  };

  // Estadísticos población 2
  const estadisticos2 = useMemo(() => {
    if (datos2.length === 0) return null;
    const media = calcMedia(datos2);
    const varianza = calcVarianza(datos2, media);
    const desviacion = calcDesviacion(varianza);
    return { media, varianza, desviacion, n: datos2.length };
  }, [datos2]);

  // Diferencia de medias
  const calcularDiferenciaMedias = () => {
    if (!estadisticos || !estadisticos2) return null;
    const { media: m1, desviacion: s1, n: n1 } = estadisticos;
    const { media: m2, desviacion: s2, n: n2 } = estadisticos2;
    const diferencia = m1 - m2;
    const errorEstandar = Math.sqrt((s1 * s1) / n1 + (s2 * s2) / n2);
    const z = getZValue(nivelConfianza);
    const error = z * errorEstandar;
    const estadisticoZ = diferencia / errorEstandar;
    return { diferencia, errorEstandar, inferior: diferencia - error, superior: diferencia + error, estadisticoZ };
  };

  // Diferencia de proporciones
  const calcularDifProporciones = () => {
    const x1 = parseFloat(prop1.exitos);
    const n1 = parseFloat(prop1.n);
    const x2 = parseFloat(prop2.exitos);
    const n2 = parseFloat(prop2.n);
    if ([x1, n1, x2, n2].some(isNaN) || n1 <= 0 || n2 <= 0) return null;
    const p1 = x1 / n1;
    const p2 = x2 / n2;
    const diferencia = p1 - p2;
    const errorEstandar = Math.sqrt((p1 * (1 - p1)) / n1 + (p2 * (1 - p2)) / n2);
    const z = getZValue(nivelConfianza);
    const error = z * errorEstandar;
    const estadisticoZ = diferencia / errorEstandar;
    
    // Prueba de hipótesis (H0: p1 = p2)
    const pCombinada = (x1 + x2) / (n1 + n2);
    const errorH0 = Math.sqrt(pCombinada * (1 - pCombinada) * (1/n1 + 1/n2));
    const zPrueba = diferencia / errorH0;
    
    let pValor;
    if (hipotesisAlternativa === 'diferente') {
      pValor = 2 * (1 - normalCDF(Math.abs(zPrueba)));
    } else if (hipotesisAlternativa === 'mayor') {
      pValor = 1 - normalCDF(zPrueba);
    } else {
      pValor = normalCDF(zPrueba);
    }
    
    return { p1, p2, diferencia, errorEstandar, inferior: diferencia - error, superior: diferencia + error, 
             estadisticoZ, zPrueba, pValor };
  };

  // Simulación TLC
  const simularTLC = () => {
    if (datos.length === 0) return;
    const medias = [];
    for (let i = 0; i < tlcSimulaciones; i++) {
      let suma = 0;
      for (let j = 0; j < tlcN; j++) {
        suma += datos[Math.floor(Math.random() * datos.length)];
      }
      medias.push(suma / tlcN);
    }
    
    // Crear histograma de medias
    const min = Math.min(...medias);
    const max = Math.max(...medias);
    const numBins = 20;
    const binWidth = (max - min) / numBins;
    const bins = Array(numBins).fill(0);
    medias.forEach(m => {
      const idx = Math.min(Math.floor((m - min) / binWidth), numBins - 1);
      bins[idx]++;
    });
    
    const mediaMedias = calcMedia(medias);
    const desvMedias = calcDesviacion(calcVarianza(medias, mediaMedias));
    
    setTlcData({
      histograma: bins.map((count, i) => ({
        valor: (min + (i + 0.5) * binWidth).toFixed(2),
        frecuencia: count
      })),
      mediaMedias,
      desvMedias,
      mediaOriginal: estadisticos.media,
      desvTeorica: estadisticos.desviacion / Math.sqrt(tlcN)
    });
  };

  const icMedia = calcularICMedia();
  const icProporcion = calcularICProporcion();
  const nMedia = calcularTamanoMuestraMedia();
  const nProporcion = calcularTamanoMuestraProporcion();
  const difMedias = calcularDiferenciaMedias();
  const difProp = calcularDifProporciones();

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-black relative overflow-hidden">
      {/* Animated grid background */}
      <div className="absolute inset-0 bg-[linear-gradient(rgba(0,255,0,0.03)_1px,transparent_1px),linear-gradient(90deg,rgba(0,255,0,0.03)_1px,transparent_1px)] bg-[size:50px_50px] [mask-image:radial-gradient(ellipse_80%_80%_at_50%_50%,black,transparent)]" />
      
      {/* Glowing orbs */}
      <div className="absolute top-20 left-10 w-96 h-96 bg-green-500/10 rounded-full blur-3xl animate-pulse" />
      <div className="absolute bottom-20 right-10 w-96 h-96 bg-cyan-500/10 rounded-full blur-3xl animate-pulse" style={{ animationDelay: '1s' }} />
      
      <div className="max-w-6xl mx-auto p-4 md:p-8 relative z-10">
        {/* Header */}
        <div className="text-center mb-8">
          <div className="inline-flex items-center gap-2 bg-black/40 backdrop-blur-sm border border-green-500/30 px-4 py-2 rounded-full shadow-lg mb-4">
            <Sparkles className="w-4 h-4 text-green-400 animate-pulse" />
            <span className="text-sm text-green-300 font-mono">Proyecto Final de Estadística</span>
          </div>
          <h1 className="text-3xl md:text-5xl font-bold bg-gradient-to-r from-green-400 via-cyan-400 to-green-400 bg-clip-text text-transparent mb-2 font-mono tracking-wider">
            Calculadora Estadística
          </h1>
          <p className="text-cyan-400/70 font-mono text-sm">Análisis Completo · Inferencia Estadística Avanzada</p>
        </div>

        <Tabs defaultValue="datos" className="space-y-6">
          <TabsList className="w-full flex flex-wrap justify-center gap-2 bg-black/60 backdrop-blur-xl p-3 rounded-xl shadow-2xl border border-green-500/20">
            <TabsTrigger value="datos" className="flex items-center gap-2 font-mono text-xs data-[state=active]:bg-green-500/20 data-[state=active]:text-green-400 data-[state=active]:shadow-[0_0_15px_rgba(34,197,94,0.3)] data-[state=active]:border data-[state=active]:border-green-500/50 text-cyan-500/70 hover:text-cyan-400 transition-all">
              <Calculator className="w-4 h-4" /> Datos
            </TabsTrigger>
            <TabsTrigger value="estadisticos" className="flex items-center gap-2 font-mono text-xs data-[state=active]:bg-green-500/20 data-[state=active]:text-green-400 data-[state=active]:shadow-[0_0_15px_rgba(34,197,94,0.3)] data-[state=active]:border data-[state=active]:border-green-500/50 text-cyan-500/70 hover:text-cyan-400 transition-all">
              <BarChart3 className="w-4 h-4" /> Estadísticos
            </TabsTrigger>
            <TabsTrigger value="inferencia" className="flex items-center gap-2 font-mono text-xs data-[state=active]:bg-green-500/20 data-[state=active]:text-green-400 data-[state=active]:shadow-[0_0_15px_rgba(34,197,94,0.3)] data-[state=active]:border data-[state=active]:border-green-500/50 text-cyan-500/70 hover:text-cyan-400 transition-all">
              <TrendingUp className="w-4 h-4" /> Inferencia
            </TabsTrigger>
            <TabsTrigger value="dospoblaciones" className="flex items-center gap-2 font-mono text-xs data-[state=active]:bg-green-500/20 data-[state=active]:text-green-400 data-[state=active]:shadow-[0_0_15px_rgba(34,197,94,0.3)] data-[state=active]:border data-[state=active]:border-green-500/50 text-cyan-500/70 hover:text-cyan-400 transition-all">
              <Users className="w-4 h-4" /> Dos Poblaciones
            </TabsTrigger>
            <TabsTrigger value="tlc" className="flex items-center gap-2 font-mono text-xs data-[state=active]:bg-green-500/20 data-[state=active]:text-green-400 data-[state=active]:shadow-[0_0_15px_rgba(34,197,94,0.3)] data-[state=active]:border data-[state=active]:border-green-500/50 text-cyan-500/70 hover:text-cyan-400 transition-all">
              <Sparkles className="w-4 h-4" /> TLC
            </TabsTrigger>
            <TabsTrigger value="acerca" className="flex items-center gap-2 font-mono text-xs data-[state=active]:bg-green-500/20 data-[state=active]:text-green-400 data-[state=active]:shadow-[0_0_15px_rgba(34,197,94,0.3)] data-[state=active]:border data-[state=active]:border-green-500/50 text-cyan-500/70 hover:text-cyan-400 transition-all">
              <Info className="w-4 h-4" /> Acerca de
            </TabsTrigger>
          </TabsList>

          {/* Pestaña Datos */}
          <TabsContent value="datos">
            <Card className="border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
              <CardHeader className="border-b border-green-500/20">
                <CardTitle className="text-xl text-green-400 font-mono">
                  Ingreso de Datos
                </CardTitle>
                <CardDescription className="text-cyan-500/70 font-mono text-xs">Ingresa una lista de números separados por comas</CardDescription>
              </CardHeader>
              <CardContent className="space-y-4 pt-6">
                <div className="bg-slate-950/80 p-3 rounded-lg border border-cyan-500/20 shadow-inner">
                  <code className="text-sm text-green-400 font-mono">$ echo "10, 20, 15, 30, 25, 18, 22"</code>
                </div>
                <div className="space-y-2">
                  <Label className="text-cyan-400 font-mono text-xs">[INPUT_BUFFER]</Label>
                  <textarea 
                    value={inputDatos}
                    onChange={(e) => setInputDatos(e.target.value)}
                    placeholder="> Ingresa dataset separado por comas..."
                    className="w-full h-32 p-3 bg-slate-950/80 border border-green-500/30 rounded-lg focus:ring-2 focus:ring-green-500 focus:border-green-500 resize-none text-green-300 font-mono text-sm placeholder:text-green-700/50 shadow-[inset_0_0_20px_rgba(0,0,0,0.5)]"
                  />
                </div>
                <Button onClick={cargarDatos} className="bg-green-500/20 hover:bg-green-500/30 text-green-400 border border-green-500/50 shadow-[0_0_15px_rgba(34,197,94,0.2)] font-mono hover:shadow-[0_0_25px_rgba(34,197,94,0.4)] transition-all">
                  <ChevronRight className="w-4 h-4 mr-2" /> Cargar Datos
                </Button>
                
                {mensaje.texto && (
                  <div className={`p-3 rounded-lg border font-mono text-sm ${mensaje.tipo === 'success' ? 'bg-green-950/50 text-green-400 border-green-500/30 shadow-[0_0_20px_rgba(34,197,94,0.1)]' : 'bg-red-950/50 text-red-400 border-red-500/30'}`}>
                    {mensaje.texto}
                  </div>
                )}
                
                {datos.length > 0 && (
                  <div className="mt-4 p-4 bg-slate-950/60 rounded-lg border border-cyan-500/30 shadow-[0_0_30px_rgba(6,182,212,0.1)]">
                    <div className="flex items-center gap-2 mb-2">
                      <Badge variant="secondary" className="bg-cyan-500/20 text-cyan-400 border border-cyan-500/30 font-mono">
                        LENGTH: {datos.length}
                      </Badge>
                    </div>
                    <p className="text-sm text-green-400 font-mono">
                      <span className="text-cyan-500">[0...{Math.min(7, datos.length-1)}]</span> → {datos.slice(0, 8).join(', ')}{datos.length > 8 ? '...' : ''}
                    </p>
                  </div>
                )}
              </CardContent>
            </Card>
          </TabsContent>

          {/* Pestaña Estadísticos */}
          <TabsContent value="estadisticos">
            {estadisticos ? (
              <div className="grid md:grid-cols-2 gap-6">
                <Card className="border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-green-500/20">
                    <CardTitle className="text-lg text-green-400 font-mono">
                      Medidas de Tendencia Central
                    </CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-3 pt-6">
                    <StatRow label="Media (x̄)" value={estadisticos.media.toFixed(4)} />
                    <StatRow label="Mediana" value={estadisticos.mediana.toFixed(4)} />
                    <StatRow label="Moda" value={estadisticos.modas.length > 3 ? 'Multimodal' : estadisticos.modas.join(', ')} />
                  </CardContent>
                </Card>
                
                <Card className="border border-cyan-500/20 shadow-2xl shadow-cyan-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-cyan-500/20">
                    <CardTitle className="text-lg text-cyan-400 font-mono">
                      Medidas de Dispersión
                    </CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-3 pt-6">
                    <StatRow label="Varianza (s²)" value={estadisticos.varianza.toFixed(4)} />
                    <StatRow label="Desviación Estándar (s)" value={estadisticos.desviacion.toFixed(4)} />
                    <StatRow label="Error Estándar" value={estadisticos.errorEstandar.toFixed(4)} />
                    <StatRow label="Rango" value={estadisticos.rango.toFixed(4)} />
                    <StatRow label="Mínimo" value={estadisticos.minimo.toFixed(4)} />
                    <StatRow label="Máximo" value={estadisticos.maximo.toFixed(4)} />
                  </CardContent>
                </Card>

                <Card className="md:col-span-2 border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-green-500/20">
                    <CardTitle className="text-lg text-green-400 font-mono">
                      Histograma
                    </CardTitle>
                  </CardHeader>
                  <CardContent className="pt-6">
                    <div className="h-64 bg-slate-950/60 rounded-lg p-4 border border-green-500/10">
                      <ResponsiveContainer width="100%" height="100%">
                        <BarChart data={histogramaData}>
                          <CartesianGrid strokeDasharray="3 3" stroke="#22c55e20" />
                          <XAxis dataKey="rango" tick={{ fontSize: 11, fill: '#22d3ee' }} stroke="#22d3ee40" />
                          <YAxis tick={{ fill: '#22d3ee' }} stroke="#22d3ee40" />
                          <Tooltip 
                            contentStyle={{ 
                              background: '#020617', 
                              border: '1px solid #22c55e40', 
                              borderRadius: '8px',
                              color: '#22d3ee',
                              fontFamily: 'monospace'
                            }}
                          />
                          <Bar dataKey="frecuencia" fill="#22c55e" radius={[4, 4, 0, 0]} opacity={0.8} />
                        </BarChart>
                      </ResponsiveContainer>
                    </div>
                  </CardContent>
                </Card>
              </div>
            ) : (
              <EmptyState message="Primero ingresa los datos en la pestaña 'Datos'" />
            )}
          </TabsContent>

          {/* Pestaña Inferencia */}
          <TabsContent value="inferencia">
            <div className="space-y-6">
              <Card className="border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
                <CardHeader className="border-b border-green-500/20">
                  <CardTitle className="text-lg text-green-400 font-mono">
                    Nivel de Confianza
                  </CardTitle>
                </CardHeader>
                <CardContent className="pt-6">
                  <Select value={nivelConfianza} onValueChange={setNivelConfianza}>
                    <SelectTrigger className="w-48 bg-slate-950/80 border-cyan-500/30 text-cyan-400 font-mono">
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent className="bg-slate-950 border-cyan-500/30">
                      <SelectItem value="0.90" className="text-cyan-400 font-mono">90% confidence</SelectItem>
                      <SelectItem value="0.95" className="text-cyan-400 font-mono">95% confidence</SelectItem>
                      <SelectItem value="0.99" className="text-cyan-400 font-mono">99% confidence</SelectItem>
                    </SelectContent>
                  </Select>
                </CardContent>
              </Card>

              <div className="grid md:grid-cols-2 gap-6">
                {/* IC de la Media */}
                <Card className="border border-cyan-500/20 shadow-2xl shadow-cyan-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-cyan-500/20">
                    <CardTitle className="text-lg text-cyan-400 font-mono">Intervalo de Confianza de la Media</CardTitle>
                    <CardDescription className="text-cyan-500/50 font-mono text-xs">Basado en los datos ingresados</CardDescription>
                  </CardHeader>
                  <CardContent className="pt-6">
                    {icMedia ? (
                      <div className="space-y-4">
                        <div className="p-4 bg-blue-950/50 rounded-lg border border-blue-500/30 shadow-[0_0_20px_rgba(59,130,246,0.1)]">
                          <p className="text-sm text-blue-400 mb-1 font-mono">Usando Z (σ conocida)</p>
                          <p className="font-mono text-lg text-green-400">
                            ({icMedia.icZ.inferior.toFixed(4)}, {icMedia.icZ.superior.toFixed(4)})
                          </p>
                          <p className="text-xs text-cyan-500/70 mt-1 font-mono">Error: ±{icMedia.icZ.error.toFixed(4)}</p>
                        </div>
                        <div className="p-4 bg-purple-950/50 rounded-lg border border-purple-500/30 shadow-[0_0_20px_rgba(168,85,247,0.1)]">
                          <p className="text-sm text-purple-400 mb-1 font-mono">Usando t (σ desconocida)</p>
                          <p className="font-mono text-lg text-green-400">
                            ({icMedia.icT.inferior.toFixed(4)}, {icMedia.icT.superior.toFixed(4)})
                          </p>
                          <p className="text-xs text-cyan-500/70 mt-1 font-mono">Error: ±{icMedia.icT.error.toFixed(4)}</p>
                        </div>
                      </div>
                    ) : (
                      <p className="text-red-400/70 font-mono text-sm">Ingresa datos primero</p>
                    )}
                  </CardContent>
                </Card>

                {/* IC de Proporción */}
                <Card className="border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-green-500/20">
                    <CardTitle className="text-lg text-green-400 font-mono">Intervalo de Confianza de una Proporción</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4 pt-6">
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <Label className="text-cyan-400/70 font-mono text-xs">Proporción (p̂)</Label>
                        <Input 
                          type="number" 
                          step="0.01" 
                          min="0" 
                          max="1"
                          placeholder="0.5"
                          value={proporcion}
                          onChange={(e) => setProporcion(e.target.value)}
                          className="bg-slate-950/80 border-green-500/30 text-green-400 font-mono focus:ring-green-500"
                        />
                      </div>
                      <div>
                        <Label className="text-cyan-400/70 font-mono text-xs">Tamaño muestra (n)</Label>
                        <Input 
                          type="number" 
                          placeholder="100"
                          value={tamanoMuestra}
                          onChange={(e) => setTamanoMuestra(e.target.value)}
                          className="bg-slate-950/80 border-green-500/30 text-green-400 font-mono focus:ring-green-500"
                        />
                      </div>
                    </div>
                    {icProporcion && (
                      <div className="p-4 bg-green-950/50 rounded-lg border border-green-500/30 shadow-[0_0_20px_rgba(34,197,94,0.1)]">
                        <p className="font-mono text-lg text-green-400">
                          [{icProporcion.inferior.toFixed(4)}, {icProporcion.superior.toFixed(4)}]
                        </p>
                        <p className="text-xs text-cyan-500/70 mt-1 font-mono">Error: ±{icProporcion.error.toFixed(4)}</p>
                      </div>
                    )}
                  </CardContent>
                </Card>

                {/* Tamaño de Muestra para Media */}
                <Card className="border border-amber-500/20 shadow-2xl shadow-amber-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-amber-500/20">
                    <CardTitle className="text-lg text-amber-400 font-mono">Tamaño de Muestra para Media</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4 pt-6">
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <Label className="text-cyan-400/70 font-mono text-xs">Error máximo (E)</Label>
                        <Input 
                          type="number" 
                          step="0.01"
                          placeholder="0.5"
                          value={errorDeseado}
                          onChange={(e) => setErrorDeseado(e.target.value)}
                          className="bg-slate-950/80 border-amber-500/30 text-amber-400 font-mono focus:ring-amber-500"
                        />
                      </div>
                      <div>
                        <Label className="text-cyan-400/70 font-mono text-xs">Desv. poblacional (σ)</Label>
                        <Input 
                          type="number" 
                          step="0.01"
                          placeholder={estadisticos ? estadisticos.desviacion.toFixed(2) : '1'}
                          value={desviacionPoblacional}
                          onChange={(e) => setDesviacionPoblacional(e.target.value)}
                          className="bg-slate-950/80 border-amber-500/30 text-amber-400 font-mono focus:ring-amber-500"
                        />
                      </div>
                    </div>
                    {nMedia && (
                      <div className="p-4 bg-amber-950/50 rounded-lg border border-amber-500/30 shadow-[0_0_20px_rgba(245,158,11,0.1)]">
                        <p className="text-sm text-amber-400/70 mb-1 font-mono">Tamaño de muestra necesario:</p>
                        <p className="font-mono text-2xl font-bold text-amber-400">n = {nMedia}</p>
                      </div>
                    )}
                  </CardContent>
                </Card>

                {/* Tamaño de Muestra para Proporción */}
                <Card className="border border-teal-500/20 shadow-2xl shadow-teal-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-teal-500/20">
                    <CardTitle className="text-lg text-teal-400 font-mono">Tamaño de Muestra para Proporción</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4 pt-6">
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <Label className="text-cyan-400/70 font-mono text-xs">Error máximo (E)</Label>
                        <Input 
                          type="number" 
                          step="0.01"
                          placeholder="0.05"
                          value={errorDeseado}
                          onChange={(e) => setErrorDeseado(e.target.value)}
                          className="bg-slate-950/80 border-teal-500/30 text-teal-400 font-mono focus:ring-teal-500"
                        />
                      </div>
                      <div>
                        <Label className="text-cyan-400/70 font-mono text-xs">Proporción estimada</Label>
                        <Input 
                          type="number" 
                          step="0.01"
                          min="0"
                          max="1"
                          placeholder="0.5"
                          value={proporcionEstimada}
                          onChange={(e) => setProporcionEstimada(e.target.value)}
                          className="bg-slate-950/80 border-teal-500/30 text-teal-400 font-mono focus:ring-teal-500"
                        />
                      </div>
                    </div>
                    {nProporcion && (
                      <div className="p-4 bg-teal-950/50 rounded-lg border border-teal-500/30 shadow-[0_0_20px_rgba(20,184,166,0.1)]">
                        <p className="text-sm text-teal-400/70 mb-1 font-mono">Tamaño de muestra necesario:</p>
                        <p className="font-mono text-2xl font-bold text-teal-400">n = {nProporcion}</p>
                      </div>
                    )}
                  </CardContent>
                </Card>

                {/* Cálculo de Z y t */}
                <Card className="md:col-span-2 border border-purple-500/20 shadow-2xl shadow-purple-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-purple-500/20">
                    <CardTitle className="text-lg text-purple-400 font-mono">Valores Críticos Z y t</CardTitle>
                  </CardHeader>
                  <CardContent className="pt-6">
                    <div className="grid md:grid-cols-3 gap-4">
                      <div className="p-4 bg-slate-950/60 rounded-lg text-center border border-blue-500/20 shadow-[0_0_20px_rgba(59,130,246,0.1)]">
                        <p className="text-sm text-blue-400/70 font-mono mb-2">Z crítico ({(parseFloat(nivelConfianza) * 100).toFixed(0)}%)</p>
                        <p className="text-3xl font-bold text-blue-400">{getZValue(nivelConfianza)}</p>
                      </div>
                      {estadisticos && (
                        <div className="p-4 bg-slate-950/60 rounded-lg text-center border border-purple-500/20 shadow-[0_0_20px_rgba(168,85,247,0.1)]">
                          <p className="text-sm text-purple-400/70 font-mono mb-2">t crítico (gl = {estadisticos.n - 1})</p>
                          <p className="text-3xl font-bold text-purple-400">{getTValue(nivelConfianza, estadisticos.n - 1).toFixed(3)}</p>
                        </div>
                      )}
                      {estadisticos && (
                        <div className="p-4 bg-slate-950/60 rounded-lg text-center border border-cyan-500/20 shadow-[0_0_20px_rgba(6,182,212,0.1)]">
                          <p className="text-sm text-cyan-400/70 font-mono mb-2">Error Estándar</p>
                          <p className="text-3xl font-bold text-cyan-400">{estadisticos.errorEstandar.toFixed(4)}</p>
                        </div>
                      )}
                    </div>
                  </CardContent>
                </Card>
              </div>
            </div>
          </TabsContent>

          {/* Pestaña Dos Poblaciones */}
          <TabsContent value="dospoblaciones">
            <div className="space-y-6">
              {/* Ingreso datos población 2 */}
              <Card className="border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
                <CardHeader className="border-b border-green-500/20">
                  <CardTitle className="text-lg text-green-400 font-mono">Datos de la Segunda Población</CardTitle>
                  <CardDescription className="text-cyan-500/70 font-mono text-xs">Ingresa los datos de la segunda muestra para comparar</CardDescription>
                </CardHeader>
                <CardContent className="space-y-4 pt-6">
                  <textarea 
                    value={inputDatos2}
                    onChange={(e) => setInputDatos2(e.target.value)}
                    placeholder="> Ingresa datos de la segunda muestra..."
                    className="w-full h-24 p-3 bg-slate-950/80 border border-green-500/30 rounded-lg focus:ring-2 focus:ring-green-500 focus:border-green-500 resize-none text-green-300 font-mono text-sm placeholder:text-green-700/50 shadow-[inset_0_0_20px_rgba(0,0,0,0.5)]"
                  />
                  <Button onClick={cargarDatos2} className="bg-green-500/20 hover:bg-green-500/30 text-green-400 border border-green-500/50 shadow-[0_0_15px_rgba(34,197,94,0.2)] font-mono">
                    Cargar Datos Población 2
                  </Button>
                  {datos2.length > 0 && (
                    <Badge className="bg-green-500/20 text-green-400 border border-green-500/30 font-mono">
                      n₂ = {datos2.length} datos
                    </Badge>
                  )}
                </CardContent>
              </Card>

              <div className="grid md:grid-cols-2 gap-6">
                {/* Diferencia de Medias */}
                <Card className="border border-blue-500/20 shadow-2xl shadow-blue-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-blue-500/20">
                    <CardTitle className="text-lg text-blue-400 font-mono">Diferencia de Medias</CardTitle>
                  </CardHeader>
                  <CardContent className="pt-6">
                    {difMedias ? (
                      <div className="space-y-3">
                        <StatRow label="Diferencia (x̄₁ - x̄₂)" value={difMedias.diferencia.toFixed(4)} />
                        <StatRow label="Error Estándar" value={difMedias.errorEstandar.toFixed(4)} />
                        <StatRow label="Estadístico Z" value={difMedias.estadisticoZ.toFixed(4)} />
                        <div className="p-4 bg-blue-950/50 rounded-lg mt-4 border border-blue-500/30 shadow-[0_0_20px_rgba(59,130,246,0.1)]">
                          <p className="text-sm text-blue-400/70 mb-1 font-mono">IC ({(parseFloat(nivelConfianza) * 100).toFixed(0)}%)</p>
                          <p className="font-mono text-blue-400">({difMedias.inferior.toFixed(4)}, {difMedias.superior.toFixed(4)})</p>
                        </div>
                      </div>
                    ) : (
                      <p className="text-red-400/70 font-mono text-sm">Ingresa datos de ambas poblaciones</p>
                    )}
                  </CardContent>
                </Card>

                {/* Diferencia de Proporciones */}
                <Card className="border border-purple-500/20 shadow-2xl shadow-purple-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-purple-500/20">
                    <CardTitle className="text-lg text-purple-400 font-mono">Diferencia de Proporciones</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4 pt-6">
                    <div className="grid grid-cols-2 gap-4">
                      <div className="space-y-2">
                        <Label className="text-cyan-400/70 font-mono text-xs">Población 1</Label>
                        <Input 
                          type="number" 
                          placeholder="Éxitos (x₁)"
                          value={prop1.exitos}
                          onChange={(e) => setProp1({...prop1, exitos: e.target.value})}
                          className="bg-slate-950/80 border-purple-500/30 text-purple-400 font-mono focus:ring-purple-500"
                        />
                        <Input 
                          type="number" 
                          placeholder="Total (n₁)"
                          value={prop1.n}
                          onChange={(e) => setProp1({...prop1, n: e.target.value})}
                          className="bg-slate-950/80 border-purple-500/30 text-purple-400 font-mono focus:ring-purple-500"
                        />
                      </div>
                      <div className="space-y-2">
                        <Label className="text-cyan-400/70 font-mono text-xs">Población 2</Label>
                        <Input 
                          type="number" 
                          placeholder="Éxitos (x₂)"
                          value={prop2.exitos}
                          onChange={(e) => setProp2({...prop2, exitos: e.target.value})}
                          className="bg-slate-950/80 border-purple-500/30 text-purple-400 font-mono focus:ring-purple-500"
                        />
                        <Input 
                          type="number" 
                          placeholder="Total (n₂)"
                          value={prop2.n}
                          onChange={(e) => setProp2({...prop2, n: e.target.value})}
                          className="bg-slate-950/80 border-purple-500/30 text-purple-400 font-mono focus:ring-purple-500"
                        />
                      </div>
                    </div>
                    {difProp && (
                      <div className="space-y-3 pt-2">
                        <StatRow label="p̂₁" value={difProp.p1.toFixed(4)} />
                        <StatRow label="p̂₂" value={difProp.p2.toFixed(4)} />
                        <StatRow label="Diferencia (p̂₁ - p̂₂)" value={difProp.diferencia.toFixed(4)} />
                        <div className="p-4 bg-purple-950/50 rounded-lg border border-purple-500/30 shadow-[0_0_20px_rgba(168,85,247,0.1)]">
                          <p className="text-sm text-purple-400/70 mb-1 font-mono">IC ({(parseFloat(nivelConfianza) * 100).toFixed(0)}%)</p>
                          <p className="font-mono text-purple-400">({difProp.inferior.toFixed(4)}, {difProp.superior.toFixed(4)})</p>
                        </div>
                      </div>
                    )}
                  </CardContent>
                </Card>

                {/* Prueba de Hipótesis */}
                <Card className="md:col-span-2 border border-pink-500/20 shadow-2xl shadow-pink-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-pink-500/20">
                    <CardTitle className="text-lg text-pink-400 font-mono">Prueba de Hipótesis</CardTitle>
                    <CardDescription className="text-cyan-500/70 font-mono text-xs">H₀: μ₁ = μ₂ (o p₁ = p₂)</CardDescription>
                  </CardHeader>
                  <CardContent className="pt-6">
                    <div className="mb-4">
                      <Label className="text-cyan-400/70 font-mono text-xs">Hipótesis Alternativa (H₁)</Label>
                      <Select value={hipotesisAlternativa} onValueChange={setHipotesisAlternativa}>
                        <SelectTrigger className="w-64 bg-slate-950/80 border-pink-500/30 text-pink-400 font-mono">
                          <SelectValue />
                        </SelectTrigger>
                        <SelectContent className="bg-slate-950 border-pink-500/30">
                          <SelectItem value="diferente" className="text-pink-400 font-mono">μ₁ ≠ μ₂ (bilateral)</SelectItem>
                          <SelectItem value="mayor" className="text-pink-400 font-mono">μ₁ {'>'} μ₂ (cola derecha)</SelectItem>
                          <SelectItem value="menor" className="text-pink-400 font-mono">μ₁ {'<'} μ₂ (cola izquierda)</SelectItem>
                        </SelectContent>
                      </Select>
                    </div>
                    
                    <div className="grid md:grid-cols-2 gap-6">
                      {difMedias && (
                        <div className="p-4 bg-blue-950/50 rounded-lg border border-blue-500/30 shadow-[0_0_20px_rgba(59,130,246,0.1)]">
                          <h4 className="font-medium text-blue-400 mb-3 font-mono">Prueba para Medias</h4>
                          <StatRow label="Estadístico Z" value={difMedias.estadisticoZ.toFixed(4)} />
                          <div className="mt-3 p-3 bg-slate-950/80 rounded border border-blue-400/20">
                            <p className="text-sm font-mono">
                              {Math.abs(difMedias.estadisticoZ) > getZValue(nivelConfianza) 
                                ? <span className="text-red-400 font-medium">✗ Rechazar H₀</span>
                                : <span className="text-green-400 font-medium">✓ No rechazar H₀</span>
                              }
                            </p>
                          </div>
                        </div>
                      )}
                      
                      {difProp && (
                        <div className="p-4 bg-purple-950/50 rounded-lg border border-purple-500/30 shadow-[0_0_20px_rgba(168,85,247,0.1)]">
                          <h4 className="font-medium text-purple-400 mb-3 font-mono">Prueba para Proporciones</h4>
                          <StatRow label="Estadístico Z" value={difProp.zPrueba.toFixed(4)} />
                          <StatRow label="p-valor" value={difProp.pValor.toFixed(4)} />
                          <div className="mt-3 p-3 bg-slate-950/80 rounded border border-purple-400/20">
                            <p className="text-sm font-mono">
                              {difProp.pValor < (1 - parseFloat(nivelConfianza))
                                ? <span className="text-red-400 font-medium">✗ Rechazar H₀</span>
                                : <span className="text-green-400 font-medium">✓ No rechazar H₀</span>
                              }
                            </p>
                          </div>
                        </div>
                      )}
                    </div>
                  </CardContent>
                </Card>
              </div>
            </div>
          </TabsContent>

          {/* Pestaña TLC */}
          <TabsContent value="tlc">
            <div className="space-y-6">
              <Card className="border border-emerald-500/20 shadow-2xl shadow-emerald-500/10 bg-black/60 backdrop-blur-xl">
                <CardHeader className="border-b border-emerald-500/20">
                  <CardTitle className="text-lg text-emerald-400 font-mono">Teorema del Límite Central</CardTitle>
                  <CardDescription className="text-cyan-500/70 font-mono text-xs">Simulación de distribuciones muestrales</CardDescription>
                </CardHeader>
                <CardContent className="space-y-4 pt-6">
                  <div className="grid md:grid-cols-3 gap-4">
                    <div>
                      <Label className="text-cyan-400/70 font-mono text-xs">Tamaño de cada muestra (n)</Label>
                      <Input 
                        type="number" 
                        min="5"
                        max="100"
                        value={tlcN}
                        onChange={(e) => setTlcN(parseInt(e.target.value) || 30)}
                        className="bg-slate-950/80 border-emerald-500/30 text-emerald-400 font-mono focus:ring-emerald-500"
                      />
                    </div>
                    <div>
                      <Label className="text-cyan-400/70 font-mono text-xs">Número de simulaciones</Label>
                      <Input 
                        type="number" 
                        min="100"
                        max="10000"
                        value={tlcSimulaciones}
                        onChange={(e) => setTlcSimulaciones(parseInt(e.target.value) || 1000)}
                        className="bg-slate-950/80 border-emerald-500/30 text-emerald-400 font-mono focus:ring-emerald-500"
                      />
                    </div>
                    <div className="flex items-end">
                      <Button 
                        onClick={simularTLC} 
                        disabled={datos.length === 0}
                        className="bg-emerald-500/20 hover:bg-emerald-500/30 text-emerald-400 border border-emerald-500/50 shadow-[0_0_15px_rgba(16,185,129,0.2)] font-mono"
                      >
                        <Sparkles className="w-4 h-4 mr-2" /> Simular
                      </Button>
                    </div>
                  </div>
                  
                  {datos.length === 0 && (
                    <p className="text-amber-400 text-sm font-mono">⚠️ Primero ingresa datos en la pestaña "Datos"</p>
                  )}
                </CardContent>
              </Card>

              {tlcData.histograma && (
                <Card className="border border-emerald-500/20 shadow-2xl shadow-emerald-500/10 bg-black/60 backdrop-blur-xl">
                  <CardHeader className="border-b border-emerald-500/20">
                    <CardTitle className="text-lg text-emerald-400 font-mono">Distribución de Medias Muestrales</CardTitle>
                  </CardHeader>
                  <CardContent className="pt-6">
                    <div className="h-72 mb-6 bg-slate-950/60 rounded-lg p-4 border border-emerald-500/10">
                      <ResponsiveContainer width="100%" height="100%">
                        <AreaChart data={tlcData.histograma}>
                          <CartesianGrid strokeDasharray="3 3" stroke="#10b98120" />
                          <XAxis dataKey="valor" tick={{ fontSize: 10, fill: '#22d3ee' }} stroke="#22d3ee40" />
                          <YAxis tick={{ fill: '#22d3ee' }} stroke="#22d3ee40" />
                          <Tooltip 
                            contentStyle={{ 
                              background: '#020617', 
                              border: '1px solid #10b98140', 
                              borderRadius: '8px',
                              color: '#22d3ee',
                              fontFamily: 'monospace'
                            }}
                          />
                          <Area type="monotone" dataKey="frecuencia" fill="#10b981" fillOpacity={0.6} stroke="#10b981" />
                        </AreaChart>
                      </ResponsiveContainer>
                    </div>
                    
                    <div className="grid md:grid-cols-2 gap-4">
                      <div className="p-4 bg-slate-950/60 rounded-lg border border-cyan-500/20 shadow-[0_0_20px_rgba(6,182,212,0.05)]">
                        <h4 className="font-medium mb-3 text-cyan-400 font-mono">Resultados Observados</h4>
                        <StatRow label="Media de las medias (X̄)" value={tlcData.mediaMedias.toFixed(4)} />
                        <StatRow label="Desv. Est. de medias" value={tlcData.desvMedias.toFixed(4)} />
                      </div>
                      <div className="p-4 bg-slate-950/60 rounded-lg border border-emerald-500/20 shadow-[0_0_20px_rgba(16,185,129,0.05)]">
                        <h4 className="font-medium mb-3 text-emerald-400 font-mono">Valores Teóricos (TLC)</h4>
                        <StatRow label="Media poblacional (μ)" value={tlcData.mediaOriginal.toFixed(4)} />
                        <StatRow label="σ/√n (teórico)" value={tlcData.desvTeorica.toFixed(4)} />
                      </div>
                    </div>
                    
                    <div className="mt-4 p-4 bg-emerald-950/50 rounded-lg border border-emerald-500/30 shadow-[0_0_20px_rgba(16,185,129,0.1)]">
                      <p className="text-sm text-emerald-400/80 font-mono">
                        ✓ El TLC predice que la distribución de medias muestrales se aproxima a una normal 
                        con media μ y desviación estándar σ/√n
                      </p>
                    </div>
                  </CardContent>
                </Card>
              )}
            </div>
          </TabsContent>

          {/* Pestaña Acerca de */}
          <TabsContent value="acerca">
            <Card className="border border-green-500/20 shadow-2xl shadow-green-500/10 bg-black/60 backdrop-blur-xl">
              <CardHeader className="border-b border-green-500/20">
                <CardTitle className="text-xl text-green-400 font-mono">
                  Acerca de la Aplicación
                </CardTitle>
              </CardHeader>
              <CardContent className="prose prose-invert max-w-none pt-6">
                <p className="text-cyan-400/70 mb-6 font-mono text-sm">
                  Esta aplicación fue creada como proyecto final para el análisis estadístico avanzado.
                </p>
                
                <h3 className="text-lg font-semibold text-green-400 mb-3 font-mono">Funcionalidades</h3>
                
                <div className="grid md:grid-cols-2 gap-4">
                  <div className="p-4 bg-slate-950/60 rounded-lg border border-green-500/20 shadow-[0_0_20px_rgba(34,197,94,0.05)]">
                    <h4 className="font-medium text-cyan-400 mb-2 font-mono text-sm">
                      Medidas de Tendencia Central
                    </h4>
                    <ul className="text-sm text-cyan-500/70 space-y-1 font-mono">
                      <li>• Media, mediana, moda</li>
                      <li>• Varianza y desviación estándar</li>
                      <li>• Mínimo, máximo y rango</li>
                      <li>• Histograma interactivo</li>
                    </ul>
                  </div>
                  
                  <div className="p-4 bg-slate-950/60 rounded-lg border border-cyan-500/20 shadow-[0_0_20px_rgba(6,182,212,0.05)]">
                    <h4 className="font-medium text-cyan-400 mb-2 font-mono text-sm">
                      Inferencia (Una Población)
                    </h4>
                    <ul className="text-sm text-cyan-500/70 space-y-1 font-mono">
                      <li>• Error estándar</li>
                      <li>• IC de la media (Z y t)</li>
                      <li>• IC de proporciones</li>
                      <li>• Cálculo de tamaño de muestra</li>
                    </ul>
                  </div>
                  
                  <div className="p-4 bg-slate-950/60 rounded-lg border border-purple-500/20 shadow-[0_0_20px_rgba(168,85,247,0.05)]">
                    <h4 className="font-medium text-cyan-400 mb-2 font-mono text-sm">
                      Dos Poblaciones
                    </h4>
                    <ul className="text-sm text-cyan-500/70 space-y-1 font-mono">
                      <li>• Diferencia de medias</li>
                      <li>• Diferencia de proporciones</li>
                      <li>• Pruebas de hipótesis</li>
                      <li>• Cálculo de p-valor</li>
                    </ul>
                  </div>
                  
                  <div className="p-4 bg-slate-950/60 rounded-lg border border-amber-500/20 shadow-[0_0_20px_rgba(245,158,11,0.05)]">
                    <h4 className="font-medium text-cyan-400 mb-2 font-mono text-sm">
                      Características Avanzadas
                    </h4>
                    <ul className="text-sm text-cyan-500/70 space-y-1 font-mono">
                      <li>• Simulación del TLC</li>
                      <li>• Distribuciones muestrales</li>
                      <li>• Gráficos interactivos</li>
                      <li>• Comportamiento del error estándar</li>
                    </ul>
                  </div>
                </div>
              </CardContent>
            </Card>
          </TabsContent>
        </Tabs>
      </div>
    </div>
  );
}

// Componente auxiliar para mostrar estadísticos
function StatRow({ label, value }) {
  return (
    <div className="flex justify-between items-center py-2 border-b border-green-500/10 last:border-0 group hover:bg-green-500/5 px-2 -mx-2 transition-all">
      <span className="text-cyan-400/70 font-mono text-sm group-hover:text-cyan-300 transition-colors">{label}</span>
      <span className="font-mono font-medium text-green-400 text-sm group-hover:text-green-300 group-hover:shadow-[0_0_10px_rgba(34,197,94,0.3)] transition-all">{value}</span>
    </div>
  );
}

// Componente para estado vacío
function EmptyState({ message }) {
  return (
    <Card className="border border-red-500/20 shadow-2xl shadow-red-500/10 bg-black/60 backdrop-blur-xl">
      <CardContent className="py-12 text-center">
        <div className="w-16 h-16 mx-auto mb-4 bg-red-500/10 rounded-full flex items-center justify-center border border-red-500/30 shadow-[0_0_30px_rgba(239,68,68,0.2)] animate-pulse">
          <Calculator className="w-8 h-8 text-red-400" />
        </div>
        <p className="text-red-400/70 font-mono text-sm">{message}</p>
      </CardContent>
    </Card>
  );
}
