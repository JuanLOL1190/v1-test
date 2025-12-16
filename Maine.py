import streamlit as st
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
from scipy import stats

# Funciones estadísticas auxiliares
def calc_media(data):
    return np.mean(data)

def calc_varianza(data, media):
    return np.var(data, ddof=1)

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
    return t_table.get(confianza, {}).get(closest, 1.96)

# Función para calcular probabilidad normal estándar (aproximación)
def normal_cdf(z):
    return stats.norm.cdf(z)

# Streamlit UI
st.title('Calculadora Estadística')

# Pestañas
tabs = st.selectbox('Selecciona una pestaña:', ['Datos', 'Estadísticos', 'Inferencia'])

# Sección de Datos
if tabs == 'Datos':
    st.header('Ingreso de Datos')
    data_input = st.text_area('Introduce los datos (separados por coma)', '10, 20, 15, 30, 25')

    if st.button('Cargar Datos'):
        try:
            datos = list(map(float, data_input.split(',')))
            st.success(f'Datos cargados correctamente: {datos}')
        except:
            st.error('Error al procesar los datos')

# Sección de Estadísticos
elif tabs == 'Estadísticos':
    st.header('Cálculo de Estadísticos')
    if 'datos' in locals():
        media = calc_media(datos)
        varianza = calc_varianza(datos, media)
        desviacion = calc_desviacion(varianza)

        st.write(f"**Media:** {media}")
        st.write(f"**Varianza:** {varianza}")
        st.write(f"**Desviación estándar:** {desviacion}")
        
        # Histograma
        fig, ax = plt.subplots()
        ax.hist(datos, bins=10, color='blue', edgecolor='black')
        st.pyplot(fig)
    else:
        st.error('Primero ingresa los datos en la pestaña "Datos"')

# Sección de Inferencia
elif tabs == 'Inferencia':
    st.header('Intervalo de Confianza')
    
    confianza = st.selectbox('Selecciona nivel de confianza', ['0.90', '0.95', '0.99'])
    if 'datos' in locals():
        z_value = get_z_value(confianza)
        media = calc_media(datos)
        desviacion = calc_desviacion(calc_varianza(datos, media))
        error_z = z_value * (desviacion / np.sqrt(len(datos)))
        
        st.write(f"**Valor Z:** {z_value}")
        st.write(f"**Error estándar (Z):** {error_z}")
        st.write(f"**Intervalo de Confianza (Z):** ({media - error_z}, {media + error_z})")
    else:
        st.error('Primero ingresa los datos en la pestaña "Datos"')

