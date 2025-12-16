import streamlit as st
import numpy as np
import pandas as pd
from scipy import stats
from math import sqrt, ceil

# Funciones estadísticas auxiliares
def calc_media(data):
    return sum(data) / len(data)

def calc_varianza(data, media):
    return sum([(x - media) ** 2 for x in data]) / (len(data) - 1)

def calc_desviacion(varianza):
    return np.sqrt(varianza)

# Función para obtener valor Z según nivel de confianza
def get_z_value(confianza):
    z_values = {'0.90': 1.645, '0.95': 1.96, '0.99': 2.576}
    return z_values.get(confianza, 1.96)

# Función para obtener valor t (aproximado)
def get_t_value(confianza, gl):
    t_table = {
        '0.90': {10: 1.812, 20: 1.725, 30: 1.697, 50: 1.676, 100: 1.660, 1000: 1.646},
        '0.95': {10: 2.228, 20: 2.086, 30: 2.042, 50: 2.009, 100: 1.984, 1000: 1.962},
        '0.99': {10: 3.169, 20: 2.845, 30: 2.750, 50: 2.678, 100: 2.626, 1000: 2.581}
    }
    gls = [10, 20, 30, 50, 100, 1000]
    closest = min(gls, key=lambda x: abs(x - gl))
    return t_table[confianza].get(closest, 1.96)

# Función para calcular probabilidad normal estándar (aproximación)
def normal_cdf(z):
    a1, a2, a3, a4, a5 = 0.254829592, -0.284496736, 1.421413741, -1.453152027, 1.061405429
    p = 0.3275911
    sign = -1 if z < 0 else 1
    z = abs(z) / np.sqrt(2)
    t = 1.0 / (1.0 + p * z)
    y = 1.0 - ((((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t) * np.exp(-z * z)
    return 0.5 * (1.0 + sign * y)

# Función para cargar datos y mostrar estadísticos
def cargar_datos():
    data_input = st.text_area("Ingresa tus datos", placeholder="Ejemplo: 10, 20, 30, 40", height=100)
    if st.button("Cargar Datos"):
        try:
            datos = [float(x.strip()) for x in data_input.split(',') if x.strip().replace('.', '', 1).isdigit()]
            if len(datos) == 0:
                raise ValueError("Datos no válidos.")
            st.session_state.datos = datos
            st.success(f"Datos cargados correctamente: {len(datos)} elementos")
            return datos
        except Exception as e:
            st.error(f"Error: {str(e)}")
            return None

# Calculando estadísticos
def calcular_estadisticos(datos):
    media = calc_media(datos)
    varianza = calc_varianza(datos, media)
    desviacion = calc_desviacion(varianza)
    return media, varianza, desviacion

# Mostrar los resultados
def mostrar_estadisticos(estadisticos):
    media, varianza, desviacion = estadisticos
    st.write(f"**Media:** {media:.2f}")
    st.write(f"**Varianza:** {varianza:.2f}")
    st.write(f"**Desviación estándar:** {desviacion:.2f}")

# Función para el cálculo del intervalo de confianza para la media
def calcular_ic_media(datos, nivel_confianza):
    media, varianza, desviacion = calcular_estadisticos(datos)
    n = len(datos)
    z = get_z_value(nivel_confianza)
    error_z = z * (desviacion / sqrt(n))
    return (media - error_z, media + error_z, error_z)

# Función para el cálculo del intervalo de confianza para proporciones
def calcular_ic_proporcion(p, n, nivel_confianza):
    z = get_z_value(nivel_confianza)
    error = z * sqrt((p * (1 - p)) / n)
    return (max(0, p - error), min(1, p + error), error)

# Tamaño de muestra para media
def calcular_tamano_muestra_media(error_deseado, desviacion_poblacional, nivel_confianza):
    z = get_z_value(nivel_confianza)
    return ceil(((z * desviacion_poblacional) / error_deseado) ** 2)

# Tamaño de muestra para proporción
def calcular_tamano_muestra_proporcion(error_deseado, proporcion_estimada, nivel_confianza):
    z = get_z_value(nivel_confianza)
    return ceil(((z ** 2) * proporcion_estimada * (1 - proporcion_estimada)) / (error_deseado ** 2))

# Interfaz de Streamlit
def main():
    st.title("Calculadora Estadística Completa")
    
    # Sección de Cargar Datos
    datos = cargar_datos()
    
    if datos:
        # Mostrar estadísticas descriptivas
        estadisticos = calcular_estadisticos(datos)
        mostrar_estadisticos(estadisticos)
        
        # Intervalo de Confianza para la Media
        nivel_confianza = st.selectbox("Nivel de Confianza", ["0.90", "0.95", "0.99"])
        ic_media = calcular_ic_media(datos, nivel_confianza)
        st.write(f"**Intervalo de Confianza para la Media:** ({ic_media[0]:.2f}, {ic_media[1]:.2f}), Error: ±{ic_media[2]:.2f}")
        
        # Tamaño de Muestra para Media
        error_deseado = st.number_input("Error deseado (E) para la Media", min_value=0.01)
        desviacion_poblacional = st.number_input("Desviación poblacional (σ)", min_value=0.01)
        if error_deseado > 0 and desviacion_poblacional > 0:
            tamano_muestra_media = calcular_tamano_muestra_media(error_deseado, desviacion_poblacional, nivel_confianza)
            st.write(f"Tamaño de muestra necesario para la media: {tamano_muestra_media}")
        
        # Intervalo de Confianza para Proporción
        p = st.number_input("Proporción estimada (p̂)", min_value=0.0, max_value=1.0, step=0.01, value=0.5)
        n = st.number_input("Tamaño de la muestra (n)", min_value=1, step=1)
        if p and n:
            ic_proporcion = calcular_ic_proporcion(p, n, nivel_confianza)
            st.write(f"**Intervalo de Confianza para Proporción:** ({ic_proporcion[0]:.2f}, {ic_proporcion[1]:.2f}), Error: ±{ic_proporcion[2]:.2f}")
        
        # Tamaño de Muestra para Proporción
        error_deseado_proporcion = st.number_input("Error deseado (E) para Proporción", min_value=0.01)
        proporcion_estimada = st.number_input("Proporción estimada para Proporción", min_value=0.0, max_value=1.0, step=0.01, value=0.5)
        if error_deseado_proporcion > 0:
            tamano_muestra_proporcion = calcular_tamano_muestra_proporcion(error_deseado_proporcion, proporcion_estimada, nivel_confianza)
            st.write(f"Tamaño de muestra necesario para la proporción: {tamano_muestra_proporcion}")

if __name__ == "__main__":
    main()

