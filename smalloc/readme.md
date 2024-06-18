__SMalloc Memory Management Library__

## Overview
SMalloc는 다양한 메모리 할당 전략을 지원하는 사용자 정의 메모리 관리 라이브러리입니다. 
이 라이브러리는 firstfit, bestfit, worstfit 전략을 포함하고 있으며, 동적 메모리 할당 및 해제, 메모리 병합(smalloc_coalesce) 기능을 제공합니다. 
이러한 기능을 통해 사용자는 메모리 할당의 효율성을 향상시킬 수 있습니다.

## Key Features
Dynamic Memory Allocation: 다양한 할당 전략(firstfit, bestfit, worstfit)을 사용하여 메모리 블록을 할당합니다.
Memory Deallocation: 할당된 메모리 블록을 해제하고, 인접한 해제된 블록과 병합하여 메모리 단편화를 줄입니다 (smalloc_coalesce).
Memory Usage Optimization: 메모리 사용 패턴에 따라 최적의 할당 전략을 선택하는 알고리즘을 개발하여 성능을 최적화합니다.

## Inefficient Aspects and Improvement Suggestions
1. Memory Search Efficiency
Current Issue: firstfit, bestfit, worstfit 전략은 순차적 검색을 통해 메모리 블록을 찾습니다. 이로 인해 메모리 리스트가 길어질수록 할당 시간이 증가합니다.
Improvement: 메모리 블록을 보다 효율적으로 관리할 수 있는 균형 이진 트리나 힙과 같은 자료 구조를 사용하여 검색 성능을 향상시킵니다.
2. Memory Fragmentation Management
Current Issue: 기본 병합 기능만을 제공하며, 복잡한 메모리 할당/해제 패턴에서는 단편화가 증가할 수 있습니다.
Improvement: 블록 크기에 따른 병합 전략을 도입하고, 할당/해제 패턴에 최적화된 병합 로직을 구현하여 단편화를 더욱 효과적으로 관리합니다.
3. Memory Allocation Performance Optimization
Improvement: 할당 요청에 따라 동적으로 메모리 할당 전략을 선택할 수 있는 알고리즘을 개발하고, 이를 사용하여 사용 패턴에 따라 최적의 전략을 적용할 수 있도록 합니다.

## Usage Scenario and Demonstration
smalloc 라이브러리를 사용하는 구체적인 시나리오와 그 시연을 통해 라이브러리의 기능을 이해하고 효율성을 검증할 수 있습니다. 이는 메모리 할당 요청 시나리오를 포함하여, 사용자 입력에 따라 메모리를 할당하고 해제하는 과정을 시뮬레이션하여 라이브러리의 반응과 성능을 평가합니다.

## Conclusion
SMalloc 라이브러리는 메모리 관리를 최적화하고, 사용자에게 다양한 메모리 할당 전략을 제공하여, 메모리 사용의 유연성과 효율성을 향상시킬 수 있습니다. 
제안된 개선 방안을 통해 라이브러리의 성능을 극대화하고, 메모리 관리를 더욱 효과적으로 수행할 수 있습니다.
