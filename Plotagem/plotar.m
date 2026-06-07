dados = readtable('dados_comparacao.csv');

dados = sortrows(dados, 'Elemento');

figure;
hold on; 
grid on; 

plot(dados.Elemento, dados.Comparacoes_FilaSimples, 'r', 'LineWidth', 2);

plot(dados.Elemento, dados.Comparacoes_Heap, 'g', 'LineWidth', 2);  

title('Desempenho de Remoção: Fila Simples vs Max-Heap');
xlabel('Número de Elementos Restantes na Estrutura');
ylabel('Quantidade de Comparações Feitas');

legend('Fila Simples (O(n))', 'Fila com Heap (O(log n))', 'Location', 'northwest');

hold off;