dados = readtable('dados_comparacao.csv');

dados = sortrows(dados, 'Quantidade_de_Elementos');

figure;
hold on; 
grid on; 

plot(dados.Quantidade_de_Elementos, dados.Comparacoes_Fila, 'r', 'LineWidth', 2);

plot(dados.Quantidade_de_Elementos, dados.Comparacoes_Heap, 'g', 'LineWidth', 2);  

title('Desempenho de Remoção: Fila Simples vs Max-Heap');
xlabel('Tamanho');
ylabel('Quantidade de Comparações Feitas');

legend('Fila Simples (O(n))', 'Fila com Heap (O(log n))', 'Location', 'northwest');

hold off;