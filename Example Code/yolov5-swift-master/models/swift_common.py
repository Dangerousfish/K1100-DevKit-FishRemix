import torch
from torch import Tensor
from models.common import *


def channel_shuffle(x: Tensor, groups: int) -> Tensor:
    batchsize, num_channels, height, width = x.size()
    channels_per_group = num_channels // groups

    # reshape
    x = x.view(batchsize, groups,
               channels_per_group, height, width)

    x = torch.transpose(x, 1, 2).contiguous()

    # flatten
    x = x.view(batchsize, -1, height, width)

    return x


# conv bn relu maxpool
class BMConv(nn.Module):
    def __init__(self, c1, c2, shortcut=True, g=1, e=0.5):  # ch_in, ch_out
        super(BMConv, self).__init__()
        self.cv1 = Conv(c1, c2, 3, 2, act=nn.ReLU(True))
        self.m = nn.MaxPool2d(kernel_size=3, stride=2, padding=1, dilation=1, ceil_mode=False)

    def forward(self, x):
        return self.m(self.cv1(x))


# ShuffleNetV2 Inverted Residual Neural Network
class ShuffleNetV2(nn.Module):
    def __init__(
            self,
            c1, c2, stride,
    ) -> None:
        super(ShuffleNetV2, self).__init__()

        if not (1 <= stride <= 3):
            raise ValueError('illegal stride value')
        self.stride = stride

        branch_features = c2 // 2
        assert (self.stride != 1) or (c1 == branch_features << 1)

        if self.stride > 1:
            self.cv1 = Conv(c1, c1, 3, self.stride, act=nn.ReLU(True), g=c1)
            self.cv2 = Conv(c1, branch_features, 1, 1)
        else:
            self.cv1 = nn.Sequential()
            self.cv2 = nn.Sequential()

        # self.m = nn.Sequential(
        #     Conv(c1 if (self.stride > 1) else branch_features, branch_features, 1, 1),
        #     Conv(branch_features, branch_features, 3, self.stride, 1, g=branch_features),
        #     Conv(branch_features, branch_features, 1, 1))
        self.cv3 = Conv(c1 if (self.stride > 1) else branch_features, branch_features, 1, 1)
        self.cv4 = Conv(branch_features, branch_features, 3, self.stride, 1, g=branch_features)
        self.cv5 = Conv(branch_features, branch_features, 1, 1)

    def forward(self, x: Tensor) -> Tensor:
        if self.stride == 1:
            x1, x2 = x.chunk(2, dim=1)
            out = torch.cat((x1, self.cv5(self.cv4(self.cv3(x2)))), dim=1)
        else:
            out = torch.cat((self.cv2(self.cv1(x)), self.cv5(self.cv4(self.cv3(x)))), dim=1)

        out = channel_shuffle(out, 2)

        return out
